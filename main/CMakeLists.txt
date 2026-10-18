idf_component_register(
    SRCS "main.c"
         "bagua_core.c"
         "bagua_data.c"
         "bagua_font.c"
         "bagua_img.c"
         "bagua_ui.c"
         "bagua_app.c"
         "bg_font_16.c"
         "bg_font_22.c"
    INCLUDE_DIRS "."
    REQUIRES bsp nvs_flash esp_timer
)
