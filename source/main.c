// Pearl Panic — point d'entrée et boucle principale (tâche 1.1)
#include <nds.h>

#include "core/input.h"
#include "core/scene.h"
#include "gfx/sprites.h"
#include "gfx/video.h"
#include "scenes/scenes.h"

int main(void)
{
    video_init();
    sprites_init();

    scene_change(&SCENE_TITLE);

    // Une itération = une frame (60 Hz)
    while (1) {
        input_update();
        scene_update();

        swiWaitForVBlank();
        sprites_commit();
    }
    return 0;
}
