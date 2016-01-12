/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "graphic/render_queue.h"

#include <algorithm>
#include <limits>

#include "base/log.h"
#include "base/rect.h"
#include "base/wexception.h"
#include "graphic/gl/blit_program.h"
#include "graphic/gl/dither_program.h"
#include "graphic/gl/draw_line_program.h"
#include "graphic/gl/fill_rect_program.h"
#include "graphic/gl/road_program.h"
#include "graphic/gl/terrain_program.h"

namespace {

constexpr int kMaximumZValue = std::numeric_limits<uint16_t>::max();
constexpr float kOpenGlZDelta = -2.f / kMaximumZValue;

// Maps [0, kMaximumZValue] linearly to [1., -1.] for use in vertex shaders.
inline float to_opengl_z(const int z) {
	return -(2.f * z) / kMaximumZValue + 1.f;
}

// The key defines in which order we render things.
//
// For opaque objects, render order makes no difference in the final image, but
//   - we batch up by program to have maximal batching.
//   - and we want to render frontmost objects first, so that we do not render
//     any pixel more than once.
static_assert(RenderQueue::Program::kHighestProgramId <= 8, "Need to change sorting keys.");  // 4 bits.

uint64_t
make_key_opaque(const uint64_t program_id, const uint64_t z_value, const uint64_t extra_value) {
	assert(program_id < RenderQueue::Program::kHighestProgramId);
	assert(z_value < std::numeric_limits<uint16_t>::max());

	// TODO(sirver): As a higher priority for sorting then z value, texture
	// could be used here. This allows for more batching of GL calls, but in my
	// tests hardly made a difference for Widelands..
	uint64_t sort_z_value = std::numeric_limits<uint16_t>::max() - z_value;
	// IIII0000 EEEEEEEE EEEEEEEE EEEEEEEE EEEEEEEE ZZZZZZZZ ZZZZZZZZ
	return (program_id << 60) | (extra_value << 16) | (sort_z_value);
}

// For blended objects, we need to render furthest away objects first, and we
// do not update the z-buffer. This guarantees that the image is correct.
//   - if z value is the same, we order by program second to have potential batching.
uint64_t
make_key_blended(const uint64_t program_id, const uint64_t z_value, const uint64_t extra_value) {
	assert(program_id < RenderQueue::Program::kHighestProgramId);
	assert(z_value < std::numeric_limits<uint16_t>::max());

	// Sort opaque objects increasing, alpha objects decreasing in order.
	// ZZZZZZZZ ZZZZZZZZ IIII0000 EEEEEEEE EEEEEEEE EEEEEEEE EEEEEEEE
	return (z_value << 40) | (program_id << 36) | extra_value;
}

// Construct 'args' used by the individual programs out of 'item'.
inline void from_item(const RenderQueue::Item& item, VanillaBlitProgram::Arguments* args) {
	args->texture = item.vanilla_blit_arguments.texture;
	args->opacity = item.vanilla_blit_arguments.opacity;
}

inline void from_item(const RenderQueue::Item& item, MonochromeBlitProgram::Arguments* args) {
	args->texture = item.monochrome_blit_arguments.texture;
	args->blend = item.monochrome_blit_arguments.blend;
}

inline void from_item(const RenderQueue::Item& item, FillRectProgram::Arguments* args) {
	args->color = item.rect_arguments.color;
}

inline void from_item(const RenderQueue::Item& item, BlendedBlitProgram::Arguments* args) {
	args->texture = item.blended_blit_arguments.texture;
	args->blend = item.blended_blit_arguments.blend;
	args->mask = item.blended_blit_arguments.mask;
}

inline void from_item(const RenderQueue::Item& item, DrawLineProgram::Arguments* args) {
	args->color = item.line_arguments.color;
	args->line_width = item.line_arguments.line_width;
}

// Batches up as many items from 'items' that have the same 'program_id'.
// Increases 'index' and returns an argument vector that can directly be passed
// to the individual program.
template <typename T>
std::vector<T> batch_up(const RenderQueue::Program program_id,
                        const std::vector<RenderQueue::Item>& items,
                        size_t* index) {
	std::vector<T> all_args;
	while (*index < items.size()) {
		const RenderQueue::Item& current_item = items.at(*index);
		if (current_item.program_id != program_id) {
			break;
		}
		all_args.emplace_back();
		T& args = all_args.back();
		args.destination_rect = current_item.destination_rect;
		args.z_value = current_item.z_value;
		args.blend_mode = current_item.blend_mode;
		from_item(current_item, &args);
		++(*index);
	}
	return all_args;
}

// Calls glScissor for the given 'rect' and enables GL_SCISSOR_TEST at
// creation. Disables GL_SCISSOR_TEST at desctruction again.
class ScopedScissor {
public:
	ScopedScissor(const FloatRect& rect);
	~ScopedScissor();

private:
	DISALLOW_COPY_AND_ASSIGN(ScopedScissor);
};

ScopedScissor::ScopedScissor(const FloatRect& rect) {
	glScissor(rect.x, rect.y, rect.w, rect.h);
	glEnable(GL_SCISSOR_TEST);
}

ScopedScissor::~ScopedScissor() {
	glDisable(GL_SCISSOR_TEST);
}

}  // namespace

RenderQueue::RenderQueue()
   : next_z_(1),
     terrain_program_(new TerrainProgram()),
     dither_program_(new DitherProgram()),
     road_program_(new RoadProgram()) {
}

// static
RenderQueue& RenderQueue::instance() {
	static RenderQueue render_queue;
	return render_queue;
}

void RenderQueue::enqueue(const Item& given_item) {
	Item* item;
	uint32_t extra_value = 0;

	switch (given_item.program_id) {
		case Program::kBlit:
		   extra_value = given_item.vanilla_blit_arguments.texture.texture_id;
		 break;

		case Program::kBlitMonochrome:
		   extra_value = given_item.monochrome_blit_arguments.texture.texture_id;
			break;

		case Program::kBlitBlended:
		   extra_value = given_item.blended_blit_arguments.texture.texture_id;
			break;

		case Program::kLine:
		   extra_value = given_item.line_arguments.line_width;
			break;

		case Program::kRect:
		case Program::kTerrainBase:
		case Program::kTerrainDither:
		case Program::kTerrainRoad:
			/* all fallthroughs intended */
			break;

		default:
		   throw wexception("Unknown given_item.program_id: %d", given_item.program_id);
	}

	if (given_item.blend_mode == BlendMode::Copy) {
		opaque_items_.emplace_back(given_item);
		item = &opaque_items_.back();
		item->z_value = to_opengl_z(next_z_);
		item->key = make_key_opaque(static_cast<uint64_t>(item->program_id), next_z_, extra_value);
	} else {
		blended_items_.emplace_back(given_item);
		item = &blended_items_.back();
		item->z_value = to_opengl_z(next_z_);
		item->key = make_key_blended(static_cast<uint64_t>(item->program_id), next_z_, extra_value);
	}
	++next_z_;
}

void RenderQueue::draw(const int screen_width, const int screen_height) {
	if (next_z_ >= kMaximumZValue) {
		throw wexception("Too many drawn layers. Ran out of z-values.");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screen_width, screen_height);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glDisable(GL_BLEND);

	std::sort(opaque_items_.begin(), opaque_items_.end());
	draw_items(opaque_items_);
	opaque_items_.clear();

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	std::sort(blended_items_.begin(), blended_items_.end());
	draw_items(blended_items_);
	blended_items_.clear();

	glDepthMask(GL_TRUE);

	next_z_ = 1;
}

void RenderQueue::draw_items(const std::vector<Item>& items) {
	size_t i = 0;
	while (i < items.size()) {
		const Item& item = items[i];
		switch (item.program_id) {
		case Program::kBlit:
			VanillaBlitProgram::instance().draw(
			   batch_up<VanillaBlitProgram::Arguments>(Program::kBlit, items, &i));
		 break;

		case Program::kBlitMonochrome:
			MonochromeBlitProgram::instance().draw(
			   batch_up<MonochromeBlitProgram::Arguments>(Program::kBlitMonochrome, items, &i));
			break;

		case Program::kBlitBlended:
			BlendedBlitProgram::instance().draw(
			   batch_up<BlendedBlitProgram::Arguments>(Program::kBlitBlended, items, &i));
			break;

		case Program::kLine:
			DrawLineProgram::instance().draw(
			   batch_up<DrawLineProgram::Arguments>(Program::kLine, items, &i));
			break;

		case Program::kRect:
			FillRectProgram::instance().draw(
			   batch_up<FillRectProgram::Arguments>(Program::kRect, items, &i));
			break;

		case Program::kTerrainBase: {
			ScopedScissor scoped_scissor(item.destination_rect);
			terrain_program_->draw(item.terrain_arguments.gametime,
			                       *item.terrain_arguments.terrains,
			                       *item.terrain_arguments.fields_to_draw,
			                       item.z_value);
			++i;
		} break;

		case Program::kTerrainDither: {
			ScopedScissor scoped_scissor(item.destination_rect);
			dither_program_->draw(item.terrain_arguments.gametime,
			                      *item.terrain_arguments.terrains,
			                      *item.terrain_arguments.fields_to_draw,
			                      item.z_value + kOpenGlZDelta);
			++i;
		} break;

		case Program::kTerrainRoad: {
			ScopedScissor scoped_scissor(item.destination_rect);
			road_program_->draw(item.terrain_arguments.renderbuffer_width,
			                    item.terrain_arguments.renderbuffer_height,
			                    *item.terrain_arguments.fields_to_draw,
			                    item.z_value + 2 * kOpenGlZDelta);
			++i;
		} break;

		default:
			throw wexception("Unknown item.program_id: %d", item.program_id);
		}
	}
}