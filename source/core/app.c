#include "core/app.h"

#include "core/input.h"
#include "core/scene.h"
#include "game/progress.h"
#include "gfx/screen.h"
#include "plat/plat.h"
#include "scenes/scenes.h"

void app_init(void)
{
    screen_init();
    progress_load();
    scene_change(&SCENE_TITLE);
}

void app_frame(void)
{
    RawInput raw;
    plat_poll(&raw);
    input_feed(&raw);
    scene_update();
    plat_wait_vblank();
    screen_present();
}
