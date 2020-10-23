struct char_info {
    ivec2 texture_position;
    ivec2 texture_size;
    ivec2 offset;
    uint chnl;
    uint page;
};

layout (std430) buffer char_infos {
    char_info ci[];
} chars;

layout (std140) uniform font_info {
    uint line_height;
    uint base;
    uvec2 dummy;
} font;

uniform sampler2D font_pages[NUM_FONT_PAGES];

vec2 calc_character_position(vec2 char_base_position, uint char_index)
{
    char_info ci = chars.ci[char_index];
    vec2 texture_size = textureSize(font_pages[ci.page], 0).xy;

    vec2 base_offset = vec2(-ci.offset.x, -ci.offset.y + font.base);
    return (char_base_position + base_offset);
}

vec2 calc_character_left(vec2 direction, uint char_index)
{
    char_info ci = chars.ci[char_index];
    vec2 texture_size = textureSize(font_pages[ci.page], 0).xy;

    return ci.texture_size.x * direction;
}

vec2 calc_character_up(vec2 direction, uint char_index)
{
    char_info ci = chars.ci[char_index];
    vec2 texture_size = textureSize(font_pages[ci.page], 0).xy;

    return -ci.texture_size.y * normalize(vec2(-direction.y, direction.x));
}
