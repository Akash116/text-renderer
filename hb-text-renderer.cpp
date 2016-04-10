#include<iostream>
#include<harfbuzz/hb.h> 
#include<harfbuzz/hb-ot.h>
#include<harfbuzz/hb-icu.h>
#include<freetype2/ft2build.h>
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_TABLES_H
#include<cairo/cairo.h>
#include<cairo/cairo-ft.h>
#include<cmath>
#include<cassert>

static hb_blob_t *get_table_from_tag(hb_face_t* /*unused*/, hb_tag_t tag, void *user_data)
{
   FT_Face ft_face = (FT_Face) user_data;
   FT_Byte *buffer;
   FT_Error error;
   FT_ULong length = 0;

   error = FT_Load_Sfnt_Table (ft_face, tag, 0, NULL, &length);
   if(error)
       return NULL;

   buffer = (FT_Byte *) malloc (length);
   if(!buffer)
       return NULL;

   error = FT_Load_Sfnt_Table (ft_face, tag, 0, buffer, &length);
   if(error)
   {
       free (buffer);
       return NULL;
   }

   hb_blob_t *blob = hb_blob_create ((const char *) buffer, length, HB_MEMORY_MODE_READONLY, buffer, free);
   return blob;
}

int main(int argc, char **argv)
{
    const char *text = argv[2];
    const char *fontname = argv[1];
    //pre defined values for debugging
    const hb_direction_t direction = HB_DIRECTION_LTR;
    const hb_script_t script = HB_SCRIPT_LATIN;
    const hb_language_t language = hb_language_from_string("en", -1);
    double point_size = 50.0;

    FT_Library ft_library;
    FT_Face ft_face;
    
    assert(!FT_Init_FreeType (&ft_library));
    assert(!FT_New_Face (ft_library, fontname, 0, &ft_face));
    //harfbuzz
    
    hb_face_t *hb_face;
    hb_font_t *hb_font;
    assert(!FT_Reference_Face(ft_face));
    hb_face = hb_face_create_for_tables (get_table_from_tag, ft_face, hb_destroy_func_t(FT_Done_Face) );
    hb_font = hb_font_create(hb_face);
    hb_ot_font_set_funcs(hb_font);
    hb_font_set_scale(hb_font, point_size*64.0, point_size*64.0);

    hb_buffer_t *buf = hb_buffer_create();
    hb_buffer_set_direction(buf, direction);
    hb_buffer_set_script(buf, script);
    hb_buffer_set_language(buf, language);

    hb_buffer_add_utf8(buf, text, -1, 0 ,-1);
    hb_shape(hb_font, buf, NULL, 0);

    unsigned int length = hb_buffer_get_length(buf);
    hb_glyph_info_t *info = hb_buffer_get_glyph_infos(buf, NULL);
    hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(buf, NULL);

    //debugging
    for(unsigned int i=0; i<length; i++)
    {
        hb_codepoint_t glyph_id = info[i].codepoint;
        int cluster = info[i].cluster;
        double x_adv = pos[i].x_advance >>6;
        double y_adv = pos[i].y_advance >>6;
        double x_offset = pos[i].x_offset >>6;
        double y_offset = pos[i].y_offset >>6;
        char glyphname[32];
        //hb_ot_get_glyph_name not defined
        hb_font_get_glyph_name (hb_font, glyph_id, glyphname, sizeof(glyphname));

        std::cout<<"Glyph: "<<glyphname<<" Cluster: "<<cluster<<"\tx_adv: "<<x_adv<<"\ty_adv: "<<y_adv<<"\tx_off: "<<x_offset<<"\ty_off: "<<y_offset<<std::endl;
    }
    
    double cur_x=0,cur_y=0;
    for(unsigned int i=0; i<length; i++)
    {
        hb_codepoint_t glyph_id = info[i].codepoint;
        int cluster = info[i].cluster;
        double pos_x = cur_x + pos[i].x_offset /64.0;
        double pos_y = cur_y + pos[i].y_offset /64.0;
        char glyphname[32];
        hb_font_get_glyph_name (hb_font, glyph_id, glyphname, sizeof(glyphname));

        std::cout<<"Glyph: "<<glyphname<<" Cluster: "<<cluster<<"\tpos_x: "<<pos_x<<"\tpos_y: "<<pos_y<<std::endl;
        if(HB_DIRECTION_IS_HORIZONTAL (hb_buffer_get_direction(buf)))
            cur_x+= pos[i].x_advance >>6;
        else
            cur_y+= pos[i].y_advance >>6;
    }
    
    double margin = 0.5* point_size;
    double width  = 2*point_size;
    double height = 2*point_size;
    for(unsigned int i=0; i<length; i++)
    {
        width  +=  pos[i].x_advance >> 6;
        height -=  (pos[i].y_advance + pos[i].y_offset) >> 6;
    }
    std::cout<<width<<" "<<height<<std::endl;

    // cairo
    cairo_surface_t *cairo_surface;
    cairo_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, ceil(width), ceil(height));
    cairo_t *cr;
    cr = cairo_create(cairo_surface);
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_translate (cr, margin, margin);

    cairo_font_face_t *cairo_face;
    cairo_face = cairo_ft_font_face_create_for_ft_face(ft_face, 0);
    cairo_set_font_face (cr, cairo_face);
    cairo_set_font_size (cr, point_size);
    cairo_font_extents_t font_extents;
    cairo_font_extents (cr, &font_extents);
    double base = (point_size - font_extents.height)/2 + font_extents.ascent;

    cairo_glyph_t *cairo_glyphs = cairo_glyph_allocate(length);

    cur_x = 0.0;
    cur_y = 0.0;
    if(HB_DIRECTION_IS_HORIZONTAL (hb_buffer_get_direction(buf)))
    {
        if(direction==HB_DIRECTION_LTR)
            cur_x = 0.0;
        else
            cur_x = 2*point_size - margin;

        cur_y = -base;
    }
    else
    {
        cur_x = 0.0;
        cur_y = -margin;
    }

    for(unsigned int i=0; i <length; i++)
    {
        cairo_glyphs[i].index = info[i].codepoint;
        cairo_glyphs[i].x = cur_x + pos[i].x_offset /64.0;
        cairo_glyphs[i].y = -cur_y - pos[i].y_offset /64.0;
        
        if(HB_DIRECTION_IS_HORIZONTAL (hb_buffer_get_direction(buf)))
            cur_x += pos[i].x_advance >>6;
        else
            cur_y += pos[i].y_advance >>6;

        std::cout<<"x: "<<cairo_glyphs[i].x<<"\ty: "<<cairo_glyphs[i].y<<"\tcur_x: "<<cur_x<<"\tcur_y: "<<cur_y<<std::endl;
    }

    cairo_show_glyphs (cr, cairo_glyphs, length);
    cairo_glyph_free (cairo_glyphs);

    cairo_surface_write_to_png (cairo_surface, "text.png");
    cairo_font_face_destroy (cairo_face);
    cairo_destroy(cr);
    cairo_surface_destroy (cairo_surface);

    hb_buffer_destroy(buf);
    hb_font_destroy(hb_font);
    hb_face_destroy(hb_face);

    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_library);

    return 0;
}
