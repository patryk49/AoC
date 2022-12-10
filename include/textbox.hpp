#pragma once

#include <raylib.h>
#include "SPL/Utils.hpp"
#include "SPL/Arrays.hpp"



template<size_t M>
struct TextBox : FiniteArray<char, M> {
	uint16_t cursor = 0;
	uint16_t selection = UINT16_MAX;

	Vector2 position = {0.f, 0.f};
	float font_size = 12.f;
	float spacing = 1.f;
	Font *font = nullptr;

	Color text_color = BLACK;
	Color cursor_color = GREEN;
	Color selection_color = BLUE;
};



// ADDING AND DELETING TEXT
template<size_t M> static
void insert(TextBox<M> &t, char c) noexcept{
	for (size_t i=t.size; i!=t.cursor; --i) t[i] = t[i-1];
	t[t.cursor] = c;
	++t.cursor;
}

template<size_t M> static
void insert(TextBox<M> &t, Span<char> s) noexcept{
	for (size_t i=t.size; i!=t.cursor; --i) t[i] = t[i-s.size];
	for (char c : s){
		t[t.cursor] = c;
		++t.cursor;
	}
}

template<size_t M> static
void delete_before(TextBox<M> &t) noexcept{
	if (t.cursor == 0) return;
	for (size_t i=t.cursor; i!=t.size; ++i) t[i-1] = t[i];
	--t.size;
	--t.cursor;
}

template<size_t M> static
void delete_before(TextBox<M> &t, size_t count) noexcept{
	if (t.cursor == 0) return;
	count = min(count, t.cursor);
	for (size_t i=t.cursor; i!=t.size; ++i) t[i-count] = t[i];
	t.size -= count;
	t.cursor -= count;
}

template<size_t M> static
void delete_after(TextBox<M> &t) noexcept{
	if (t.cursor == t.len) return;
	--t.size;
	for (size_t i=t.cursor; i!=t.size; ++i) t[i] = t[i+1];
}

template<size_t M> static
void delete_after(TextBox<M> &t, size_t count) noexcept{
	if (t.cursor == t.len) return;
	count = min(count, t.cursor);
	t.size -= count;
	for (size_t i=t.cursor; i!=t.size; ++i) t[i] = t[i+count];
}



// SELECTION RELATED
template<size_t M> static
void disable_selection(TextBox<M> &t) noexcept{ t.selection = UINT16_MAX; }

template<size_t M> static constexpr
Span<char> has_selection(const TextBox<M> &t) noexcept{ return t.selection != UINT16_MAX; }

template<size_t M> static constexpr 
Span<char> get_selection(const TextBox<M> &t) noexcept{
	return (
		t.cursor < t.selection
		? Span<char>{t.ptr+t.cursor, t.selection-t.cursor}
		: Span<char>{t.ptr+t.selection, t.cursor-t.selection}
	);
}



// TEXT RENDERING FUNCTIONS
struct DrawTextResult{
	float pos_x;
	uint32_t count;
};

DrawTextResult measure_text(
	Span<const char> text,
	const Font *font, float font_size, float spacing
) noexcept{
	float size_x = 0.f;
	const char *it = text.ptr;
	float scale_factor = font_size / font->baseSize; 

	while (it < end(text)){
		int uc_size;
		int uc = GetCodepoint(it, &uc_size);
		size_t index = GetGlyphIndex(*font, uc);
		it += uc_size;
		
		[[unlikely]] if (uc == '\n') break;

		size_x += (
			(font->glyphs[index].advanceX == 0)
			? (float)font->recs[index].width*scale_factor
			: (float)font->glyphs[index].advanceX*scale_factor
		) + spacing;
	}
	return DrawTextResult{size_x, it-text.ptr};
}


DrawTextResult draw_text(
	Span<const char> text,
	const Font *font, Color color,
	Vector2 position, float font_size, float spacing
) noexcept{
//	float old_pos_x = position.x;
	const char *it = text.ptr;
	float scale_factor = font_size / font->baseSize; 
	
	while (it < end(text)){
		int uc_size;
		int uc = GetCodepoint(it, &uc_size);
		size_t index = GetGlyphIndex(*font, uc);
		it += uc_size;
		
		[[unlikely]] if (uc == '\n') break;
		if (uc != ' ' && uc != '\t')
			DrawTextCodepoint(*font, uc, position, font_size, color);

		position.x += (
			(font->glyphs[index].advanceX == 0)
			? (float)font->recs[index].width*scale_factor
			: (float)font->glyphs[index].advanceX*scale_factor
		) + spacing;
	}
	return DrawTextResult{position.x, it-text.ptr};
}




// DRAWING FUNCTION
template<size_t M> static
void draw(const TextBox<M> &t) noexcept{
	size_t from = min(t.cursor, t.selection);

	auto[pos_x, u] = draw_text(
		slice(t, 0, from), t.font, t.text_color, t.position, t.font_size, t.spacing
	);
	
	float cursor_pos = pos_x;
	if (t.selection != UINT16_MAX || t.selection != t.cursor){
		auto[box_x, count] = measure_text(
			slice(t, from, max(t.cursor, t.selection)), t.font, t.font_size, t.spacing
		);
		if (from == t.selection) cursor_pos += box_x;
		
		DrawRectangleV(Vector2{pos_x, t.position.y}, Vector2{box_x, t.font_size}, t.selection_color);
	}

	draw_text(
		slice(t, from, t.size),
		t.font, t.text_color, Vector2{pos_x, t.position.y}, t.font_size, t.spacing
	);

//	DrawRectangleV(
//	Vector2{pos, t.position.y},
//		Vector2{t.spacing, t.font_size},
//		t.cursor_color
//	);
}




