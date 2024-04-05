//
// Created by wsmrxd on 2024/4/5.
//

#include "gui.h"

#include "lvgl/lvgl.h"

void hello_gui()
{
    lv_obj_t* obj;

    /* set screen background to white */
    lv_obj_t* scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);

    /* create label */
    obj = lv_label_create(scr);
    lv_obj_set_align(obj, LV_ALIGN_CENTER);
    lv_obj_set_height(obj, LV_SIZE_CONTENT);
    lv_obj_set_width(obj, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(obj, lv_color_black(), 0);
    lv_label_set_text(obj, "Hello World!");
}
