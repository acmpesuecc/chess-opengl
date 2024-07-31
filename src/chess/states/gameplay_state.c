#include <GLFW/glfw3.h>
#include <math.h>

#include "../../gfx/renderer.h"
#include "../../ui/imagebox.h"
#include "../../ui/textbox.h"
#include "../../ui/ui.h"
#include "../game.h"
#include "../movegen.h"
#include "../piece.h"
#include "gameplay_state.h"

#define PIECE_ANIMATION_DURATION 0.1

static Vec2i calc_board_relative_pos(Vec2i board_top_left, Vec2i board_size, Vec2i square,
                                     ChessColor plr_color)
{
    return (Vec2i){board_top_left.x + (plr_color == BLACK ? (7 - square.x) : square.x) * board_size.x / 8,
                   board_top_left.y - (plr_color == WHITE ? (7 - square.y) : square.y) * board_size.y / 8};
}

static void empty_move_list(ChessGame *game)
{
    struct ChessData *chess_data = &game->chess_data;
    free(chess_data->current_move_list.moves);
    chess_data->current_move_list = (MoveList){NULL, 0};
}

GameState *gameplay_state_init()
{
    GameState *self = (GameState *)malloc(sizeof(GameState));

    self->type = GAMEPLAY_STATE;
    self->setup = gameplay_state_setup;
    self->update = gameplay_state_update;
    self->render = gameplay_state_render;
    self->cleanup = gameplay_state_cleanup;

    return self;
};

void gameplay_state_setup(ChessGame *game)
{
    struct ChessData *chess_data = &game->chess_data;
    struct UIData *ui_data = &game->ui_data;

    Window *w = game->renderer->window;
    int width = w->width;
    int height = w->height;
    Vec2i center = {width / 2, height / 2};

    ui_data->board_size = (Vec2i){670, 670};
    ui_data->board_pos = (Vec2i){36, center.y + ui_data->board_size.y / 2};
    ui_data->mouse_down = false;
    ui_data->mouse_pos = (Vec2f){0, 0};
    ui_data->selected_piece = NULL;
    ui_data->selected_square = (Vec2i){-1, -1};
    ui_data->animating_piece = NULL;
    ui_data->animation_time = 0;

    chess_board_init(&chess_data->board);
    chess_data->current_turn = WHITE;
    chess_data->player_color = WHITE;
    chess_data->current_move_list = (MoveList){NULL, 0};

    // background
    imagebox_create(game->ui, (Vec2i){0, height}, (Vec2i){width, height}, game->bg_texture,
                    (Color3i){255, 255, 255});

    // board
    imagebox_create(game->ui, ui_data->board_pos, ui_data->board_size, game->board_texture,
                    (Color3i){255, 255, 255});

    // player icon
    imagebox_create(game->ui, (Vec2i){36, 52}, (Vec2i){45, 45}, game->player_icon_texture,
                    (Color3i){255, 255, 255});
    textbox_create(game->ui, (Vec2i){81, 52}, (Vec2i){70, 15}, (Padding){0}, "You", game->secondary_font);

    // opponent icon
    imagebox_create(game->ui, (Vec2i){661, 789}, (Vec2i){45, 45}, game->player_icon_texture,
                    (Color3i){255, 255, 255});
    textbox_create(game->ui, (Vec2i){610, 789}, (Vec2i){50, 15}, (Padding){0}, "AI", game->secondary_font);
};

void gameplay_state_update(ChessGame *game, double delta_time)
{
    Window *w = game->renderer->window;

    struct ChessData *chess_data = &game->chess_data;
    struct UIData *ui_data = &game->ui_data;

    double xpos, ypos;
    glfwGetCursorPos(w->glfw_window, &xpos, &ypos);
    ypos = w->height - ypos; // glfw has origin at top left
    ui_data->mouse_pos = (Vec2f){xpos, ypos};

    bool left_btn_pressed = glfwGetMouseButton(w->glfw_window, GLFW_MOUSE_BUTTON_LEFT);

    Vec2i board_pos = ui_data->board_pos;
    Vec2i board_size = ui_data->board_size;
    Vec2f square_size = {(float)board_size.x / 8, (float)board_size.y / 8};

    if (xpos > board_pos.x && xpos < board_pos.x + board_size.x && ypos > board_pos.y - board_size.y &&
        ypos < board_pos.y)
    {
        // mouse is inside board
        int x = ((xpos - (float)board_pos.x) / square_size.x);
        int y = 7 - (int)(((float)board_pos.y - ypos) / square_size.y);

        if (chess_data->player_color == BLACK)
        {
            x = 7 - x;
            y = 7 - y;
        }

        if (!ui_data->mouse_down && left_btn_pressed)
        {
            // mouse btn down
            ui_data->mouse_down = true;

            if (chess_data->board.squares[x][y])
            {
                // clicked on a piece
                ui_data->selected_square = (Vec2i){x, y};
                ui_data->selected_piece = chess_data->board.squares[x][y];
                chess_data->current_move_list =
                    generate_moves(ui_data->selected_piece, (Vec2i){x, y}, &chess_data->board);
            }
            else
            {
                if (ui_data->selected_piece)
                {
                    for (int i = 0; i < chess_data->current_move_list.n_moves; i++)
                    {
                        ChessMove move = chess_data->current_move_list.moves[i];
                        if (move.to.x == x && move.to.y == y)
                        {
                            chess_board_move_piece(&chess_data->board, ui_data->selected_piece,
                                                   ui_data->selected_square, move.to);
                            chess_data->current_turn = chess_data->current_turn == WHITE ? BLACK : WHITE;
                            empty_move_list(game);

                            // animate piece
                            ui_data->animating_piece = ui_data->selected_piece;
                            ui_data->animating_from = ui_data->selected_square;
                            ui_data->animating_to = move.to;
                            ui_data->animation_time = 0;

                            break;
                        }
                    }
                }
                ui_data->selected_square = (Vec2i){-1, -1};
                ui_data->selected_piece = NULL;
            }
        }
        else if (ui_data->mouse_down && !left_btn_pressed)
        {
            // clicked and released
            ui_data->mouse_down = false;

            if (ui_data->selected_piece && !chess_data->board.squares[x][y])
            {
                // move piece
                for (int i = 0; i < chess_data->current_move_list.n_moves; i++)
                {
                    ChessMove move = chess_data->current_move_list.moves[i];
                    if (move.to.x == x && move.to.y == y)
                    {
                        chess_board_move_piece(&chess_data->board, ui_data->selected_piece,
                                               ui_data->selected_square, move.to);
                        chess_data->current_turn = chess_data->current_turn == WHITE ? BLACK : WHITE;
                        empty_move_list(game);
                        ui_data->selected_square = (Vec2i){-1, -1};
                        ui_data->selected_piece = NULL;

                        break;
                    }
                }
            }
        }
    }
    else
    {
        if (!ui_data->mouse_down && left_btn_pressed)
        {
            // user clicked outside board
            ui_data->selected_square = (Vec2i){-1, -1};
            ui_data->selected_piece = NULL;
            empty_move_list(game);
        }
        else if (ui_data->mouse_down && !left_btn_pressed)
        {
            // user released mouse outside board
            ui_data->mouse_down = false;
        }
    }

    // update animation
    if (ui_data->animating_piece)
    {
        ui_data->animation_time += 1 / 60.0;
        if (ui_data->animation_time >= PIECE_ANIMATION_DURATION)
        {
            ui_data->animating_piece = NULL;
            ui_data->animation_time = 0;
        }
    }
};

void gameplay_state_render(ChessGame *game)
{
    Renderer *r = game->renderer;
    Window *w = r->window;
    struct ChessData *chess_data = &game->chess_data;
    struct UIData *ui_data = &game->ui_data;

    int width = w->width;
    int height = w->height;

    Vec2i board_size = ui_data->board_size;
    Vec2i board_pos = ui_data->board_pos;

    ChessColor plr_color = chess_data->player_color;

    Vec2f square_size = {(float)board_size.x / 8, (float)board_size.y / 8};

    // selected square
    if (ui_data->selected_square.x != -1)
    {

        Vec2i square_pos =
            calc_board_relative_pos(board_pos, board_size, ui_data->selected_square, plr_color);

        renderer_draw_rect(r, (Color3i){255, 0, 0}, square_pos,
                           (Vec2i){ceil(square_size.x), ceil(square_size.y)});
    }

    // pieces
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            ChessPiece *piece = chess_data->board.squares[i][j];
            if (piece != NULL)
            {
                if ((ui_data->mouse_down && piece == ui_data->selected_piece) ||
                    piece == ui_data->animating_piece)
                    continue;

                Texture tex = game->piece_textures[piece->type + (piece->color == BLACK ? 6 : 0)];
                Vec2i piece_pos = calc_board_relative_pos(board_pos, board_size, (Vec2i){i, j}, plr_color);

                renderer_draw_rect_tex(r, tex, piece_pos, (Vec2i){square_size.x, square_size.y});
            }
        }
    }

    ChessPiece *selected_piece = ui_data->selected_piece;

    // selected piece moves
    if (selected_piece && chess_data->current_move_list.n_moves > 0)
    {
        for (int i = 0; i < chess_data->current_move_list.n_moves; i++)
        {
            ChessMove move = chess_data->current_move_list.moves[i];
            Vec2i square_pos = calc_board_relative_pos(board_pos, board_size, move.to, plr_color);

            renderer_draw_rect(r, (Color3i){0, 255, 0}, square_pos,
                               (Vec2i){ceil(square_size.x), ceil(square_size.y)});
        }
    }

    // currently moving piece
    if (ui_data->animating_piece)
    {
        Vec2i from = calc_board_relative_pos(board_pos, board_size, ui_data->animating_from, plr_color);
        Vec2i to = calc_board_relative_pos(board_pos, board_size, ui_data->animating_to, plr_color);

        Vec2i piece_pos = (Vec2i){from.x + (to.x - from.x) * ui_data->animation_time / PIECE_ANIMATION_DURATION,
                                  from.y + (to.y - from.y) * ui_data->animation_time / PIECE_ANIMATION_DURATION};

        Texture tex = game->piece_textures[ui_data->animating_piece->type +
                                           (ui_data->animating_piece->color == BLACK ? 6 : 0)];

        renderer_draw_rect_tex(r, tex, piece_pos, (Vec2i){square_size.x, square_size.y});
    }

    // dragged piece
    if (ui_data->mouse_down && selected_piece)
    {
        Texture tex = game->piece_textures[selected_piece->type + (selected_piece->color == BLACK ? 6 : 0)];
        Vec2i piece_pos =
            (Vec2i){ui_data->mouse_pos.x - square_size.x / 2, ui_data->mouse_pos.y + square_size.y / 2};

        renderer_draw_rect_tex(r, tex, piece_pos, (Vec2i){square_size.x, square_size.y});
    }
};

void gameplay_state_cleanup(ChessGame *game) { ui_destroy_all(game->ui); };