#include "chess/game.h"
#include "gfx/window.h"
#include "gfx/renderer.h"

int main()
{
    Window w;
    window_init(&w, 840, 800, "Chess");

    Renderer r;
    renderer_init(&r, &w);

    ChessGame game;
    game_init(&game, &r);
    game_start(&game);

    // clean up
    renderer_terminate(&r);
    window_destroy(&w);

    return 0;
}
