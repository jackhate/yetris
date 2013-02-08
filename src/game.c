
#include <stdbool.h>
#include "game.h"
#include "piece.h"
#include "board.h"
#include "engine.h"

/** Initializes and returns a new game structure with all it's dependencies */
game_s new_game()
{
	game_s g;
	int i;

	g.board = new_board();

	g.piece_current = new_piece(piece_get_random());
	for (i = 0; i < NEXT_PIECES_NO; i++)
		g.piece_next[i] = new_piece(piece_get_random());


	g.can_hold = true;
	g.piece_hold = new_piece(PIECE_DUMMY); /* create a dummy piece */
	g.score = 0;
	g.lines = 0;
	g.level = 0;

	game_ghost_update(&g);

	return g;
}

/** Refreshes the ghost piece to the current one on #g */
void game_ghost_update(game_s* g)
{
	g->piece_ghost = g->piece_current;
	g->piece_ghost.color = engine_get_color(WHITE_BLACK, false);
	int i;
	for (i = 0; i < 4; i++)
	{
		g->piece_ghost.block[i].type     = EMPTY;
		g->piece_ghost.block[i].color    = g->piece_ghost.color;
		g->piece_ghost.block[i].theme[0] = '[';
		g->piece_ghost.block[i].theme[1] = ']';
		g->piece_ghost.block[i].theme[2] = '\0';
	}
	piece_hard_drop(&(g->piece_ghost), &(g->board));
}

/** Places the current piece on the board and gets a new one. */
void game_drop_piece(game_s* g)
{
	board_save_piece(&(g->board), &(g->piece_current));

	g->piece_current = g->piece_next[0];
	int i;
	for (i = 0; i < NEXT_PIECES_NO - 1; i++)
		g->piece_next[i] = g->piece_next[i + 1];

	g->piece_next[i] = new_piece(piece_get_random());

	game_ghost_update(g);

	g->can_hold = true; /* now we can switch pieces! */
	g->score += 10;
}

/** Tests if the game is over */
bool game_is_over(game_s* g)
{
	return board_is_full(&(g->board));
}

/** Perform any updates on the data structures inside #g */
void game_update(game_s* g)
{
	game_delete_possible_lines(g);
	game_ghost_update(g);
}

bool game_hold_piece(game_s* g)
{
	if (!g->can_hold)
		return false;

   	g->can_hold = false;

	piece_s tmp = g->piece_hold;
	g->piece_hold = g->piece_current;

	if (tmp.type == PIECE_DUMMY)
	{
		g->piece_current = g->piece_next[0];
		int i;
		for (i = 0; i < NEXT_PIECES_NO - 1; i++)
			g->piece_next[i] = g->piece_next[i + 1];

		g->piece_next[i] = new_piece(piece_get_random());
		game_ghost_update(g);
	}
	else
	{
		g->piece_current = tmp;
		piece_reset(&(g->piece_current));
	}

	/* Move the hold piece to fit the hold screen */
	int k;
	for (k = 0; k < 4; k++)
	{
		g->piece_hold.block[k].x -= g->piece_hold.x;
		g->piece_hold.block[k].y -= g->piece_hold.y;
	}

	return true;
}

/** Checks all lines, deleting the ones that are full.
 *
 *  @note I know this function's ugly...
 *  @todo Maybe create a 'Line' data structure? To make this simpler?
 */
void game_delete_possible_lines(game_s* g)
{
	board_s* b = &(g->board);

	bool lines[BOARD_HEIGHT]; /* this will mark lines to be deleted */
	int  count = 0;

	int j;
	for (j = 0; j < BOARD_HEIGHT; j++)
	{
		int i = 0;
		while (i < BOARD_WIDTH)
		{
			if (b->block[i][j].type == EMPTY)
			{
				lines[j] = false;
				break;
			}
			i++;
		}
		if (i == BOARD_WIDTH)
		{
			lines[j] = true;
			count++;
		}
	}
	if (count == 0)
		return;

	board_delete_lines(b, lines);
	g->score += (count * 30);
}

