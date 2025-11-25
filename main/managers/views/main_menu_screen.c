#include "managers/views/main_menu_screen.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "lvgl.h"
#include "managers/views/app_gallery_screen.h"
#include <stdio.h>

// Declare ghost_poweroff() from main.c
void ghost_poweroff(void);

static const char *TAG = "MainMenu";

lv_obj_t *menu_container;
static int selected_item_index = 0;

typedef struct {
  const char *name;
  const lv_img_dsc_t *icon;
  lv_color_t border_color;
} menu_item_t;

// Define colors as compile-time constants
static menu_item_t menu_items[] = {
    {"BLE", &bluetooth},
    {"WiFi", &wifi},
    {"GPS", &Map},
    {"Apps", &GESPAppGallery},
    {"Power Off", NULL}  // ★ NEW MENU ITEM
};

static int num_items = sizeof(menu_items) / sizeof(menu_items[0]);
lv_obj_t *current_item_obj = NULL;

static void init_menu_colors(void) {
    // Initialize colors at runtime
    menu_items[0].border_color = lv_color_hex(0x1976D2);
    menu_items[1].border_color = lv_color_hex(0xD32F2F);
    menu_items[2].border_color = lv_color_hex(0x388E3C);
    menu_items[3].border_color = lv_color_hex(0x7B1FA2);
    menu_items[4].border_color = lv_color_hex(0x444444); // ★ Power Off color
}

// Animation callback wrapper
static void anim_set_x(void *obj, int32_t v) {
    lv_obj_set_x((lv_obj_t *)obj, (lv_coord_t)v);
}

static void update_menu_item(bool slide_left) {
    if (current_item_obj) lv_obj_del(current_item_obj);

    current_item_obj = lv_btn_create(menu_container);
    lv_obj_set_style_bg_color(current_item_obj, lv_color_hex(0x1E1E1E), LV_PART_MAIN);
    lv_obj_set_style_border_width(current_item_obj, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(current_item_obj, menu_items[selected_item_index].border_color, LV_PART_MAIN);
    lv_obj_set_style_radius(current_item_obj, 10, LV_PART_MAIN);

    int btn_size = LV_MIN(LV_HOR_RES, LV_VER_RES) * 0.6;
    lv_obj_set_size(current_item_obj, btn_size, btn_size);
    lv_obj_align(current_item_obj, LV_ALIGN_CENTER, 0, 0);

    if (menu_items[selected_item_index].icon != NULL) {
        lv_obj_t *icon = lv_img_create(current_item_obj);
        lv_img_set_src(icon, menu_items[selected_item_index].icon);
        lv_obj_center(icon);
    }

    lv_obj_t *label = lv_label_create(current_item_obj);
    lv_label_set_text(label, menu_items[selected_item_index].name);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -5);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, current_item_obj);
    lv_anim_set_time(&a, 75);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    int start_x = slide_left ? LV_HOR_RES : -LV_HOR_RES;
    lv_anim_set_values(&a, start_x, 0);
    lv_anim_set_exec_cb(&a, anim_set_x);
    lv_anim_start(&a);
}

static void select_menu_item(int index, bool slide_left) {
    if (index < 0) index = num_items - 1;
    if (index >= num_items) index = 0;
    selected_item_index = index;
    update_menu_item(slide_left);
}

static void handle_menu_item_selection(int item_index) {
    switch (item_index) {
        case 0: printf("BLE selected\n"); SelectedMenuType = OT_Bluetooth; display_manager_switch_view(&options_menu_view); break;
        case 1: printf("Wi-Fi selected\n"); SelectedMenuType = OT_Wifi; display_manager_switch_view(&options_menu_view); break;
        case 2: printf("GPS selected\n"); SelectedMenuType = OT_GPS; display_manager_switch_view(&options_menu_view); break;
        case 3: printf("Apps View Selected\n"); display_manager_switch_view(&apps_menu_view); break;

        case 4: 
            printf("Power Off selected\n");
            ghost_poweroff();  // ★ NEW FUNCTION CALL
            break;

        default:
            printf("Unknown menu item\n");
            break;
    }
}

static void menu_item_event_handler(InputEvent *event) {
    // (unchanged)
}

void main_menu_create(void) {
    display_manager_fill_screen(lv_color_hex(0x121212));
    init_menu_colors();

    menu_container = lv_obj_create(lv_scr_act());
    main_menu_view.root = menu_container;
    lv_obj_set_size(menu_container, LV_HOR_RES, LV_VER_RES);

    selected_item_index = 0;
    update_menu_item(false);

    display_manager_add_status_bar("Main Menu");
}

void main_menu_destroy(void) {
    if (menu_container) {
        lv_obj_clean(menu_container);
        lv_obj_del(menu_container);
        menu_container = NULL;
        current_item_obj = NULL;
    }
}

void get_main_menu_callback(void **callback) {
    *callback = main_menu_view.input_callback;
}

View main_menu_view = {
    .root = NULL,
    .create = main_menu_create,
    .destroy = main_menu_destroy,
    .input_callback = menu_item_event_handler,
    .name = "Main Menu",
    .get_hardwareinput_callback = get_main_menu_callback
};
