#pragma once

#include "gui/modules/variable_item_list.h"
#include "helpers/ducky_script_i.h"


#define BAD_USB_APP_BASE_FOLDER        EXT_PATH("badusb")
#define BAD_USB_APP_PATH_LAYOUT_FOLDER BAD_USB_APP_BASE_FOLDER "/assets/layouts"
#define BAD_USB_APP_SCRIPT_EXTENSION   ".txt"
#define BAD_USB_APP_LAYOUT_EXTENSION   ".kl"

typedef struct Badmouse {
    VariableItemList* var_item_list;

    FuriString* file_path;
    FuriString* keyboard_layout;
    BadUsbScript* bad_usb_script;

} Badmouse;