/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "graphic/font_handler.h"

#include <memory>

#include <boost/lexical_cast.hpp>

#include "graphic/text/rt_render.h"
#include "graphic/text/texture_cache.h"

namespace {

// The CacheSize constexpr values control the cache sizes for our transient caches.
// The idea is that they are big enough to cache the text that is used on a typical screen - so
// that we do not need to lay out text every frame.

// TODO(GunChleoc): Re-evaulate the cache sizes once the in-game help and the campaigns have been
// converted to this font renderer.
// The current cache sizes allow for loading 22 maps in the map selection screen before they start
// dropping entries. In-game dropping of textures only started after quite a lot of text was
// generated by mousing over things. Typing 500+ characters in a Multilineeditbox did not trigger
// texture dropping.

// The size of the richtext surface cache in bytes.
constexpr uint32_t kTextureCacheSize = 3 << 20;  // shifting by 20 converts to MB

// The maximum number of RenderedRects. It's all pointers or combinations of basic data types, so
// the size requirement is pretty constant. Therefore, simply counting them is sufficient.
// We estimate that the member variables of each RenderedRect take up ca. 13 * 32 bytes.
constexpr uint32_t kRenderCacheSize = 8 * 1024;
}  // namespace

namespace UI {

// Utility class to render a rich text string. The returned string is cached in
// the ImageCache, so repeated calls to render with the same arguments should not
// be a problem.
class FontHandler : public IFontHandler {
private:
	// A transient cache for the generated rendered texts
	class RenderCache : public TransientCache<RenderedText> {
	public:
		explicit RenderCache(uint32_t max_number_of_rects)
		   : TransientCache<RenderedText>(max_number_of_rects) {
		}

		std::shared_ptr<const RenderedText>
		insert(const std::string& hash, std::shared_ptr<const RenderedText> entry) override {
			return TransientCache<RenderedText>::insert(hash, entry, entry->rects.size());
		}
	};

public:
	FontHandler(ImageCache* image_cache, const std::string& locale)
	   : texture_cache_(new TextureCache(kTextureCacheSize)),
	     render_cache_(new RenderCache(kRenderCacheSize)),
	     fontsets_(),
	     fontset_(fontsets_.get_fontset(locale)),
	     rt_renderer_(new RT::Renderer(image_cache, texture_cache_.get(), fontsets_)),
	     image_cache_(image_cache) {
	}
	~FontHandler() override {
	}

	// This will render the 'text' with a width restriction of 'w'. If 'w' == 0, no restriction is
	// applied.
	std::shared_ptr<const UI::RenderedText> render(const std::string& text,
	                                               uint16_t w = 0) override {
		const std::string hash = boost::lexical_cast<std::string>(w) + text;
		std::shared_ptr<const RenderedText> rendered_text = render_cache_->get(hash);
		if (rendered_text == nullptr) {
			rendered_text = render_cache_->insert(hash, rt_renderer_->render(text, w));
		}
		return rendered_text;
	}

	UI::FontSet const* fontset() const override {
		return fontset_;
	}

	void reinitialize_fontset(const std::string& locale) override {
		fontset_ = fontsets_.get_fontset(locale);
		texture_cache_->flush();
		render_cache_->flush();
		rt_renderer_.reset(new RT::Renderer(image_cache_, texture_cache_.get(), fontsets_));
	}

private:
	std::unique_ptr<TextureCache> texture_cache_;
	std::unique_ptr<RenderCache> render_cache_;
	UI::FontSets fontsets_;       // All fontsets
	UI::FontSet const* fontset_;  // The currently active FontSet
	std::unique_ptr<RT::Renderer> rt_renderer_;
	ImageCache* const image_cache_;  // not owned
};

IFontHandler* create_fonthandler(ImageCache* image_cache, const std::string& locale) {
	return new FontHandler(image_cache, locale);
}

IFontHandler* g_fh = nullptr;

}  // namespace UI
