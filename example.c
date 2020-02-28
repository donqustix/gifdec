#include <SDL2/SDL.h>

#include "gifdec.h"

int main(int argc, char *argv[])
{
	gd_GIF *gif = gd_open_gif(argv[1]);
	int ret_code = 1;
	if (!gif) {
		fprintf(stderr, "Couldn't open %s\n", argv[1]);
		return ret_code;
	}
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		goto cleanup_gif;
	}
	SDL_Window* window;
	SDL_Renderer* renderer;
	if (SDL_CreateWindowAndRenderer(gif->width, gif->height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE, &window, &renderer)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
		goto cleanup_sdl;
	}
    SDL_SetWindowTitle(window, "gifwallpaper");
	Uint8 *frame = malloc(gif->width * gif->height * 3);
	if (!frame) {
		fprintf(stderr, "Couldn't allocate frame\n");
		goto cleanup_window_renderer;
	}
	struct TextureNode {
		SDL_Texture* texture;
		struct TextureNode* next;
	} *texture_node_root;
	for (struct TextureNode **texture_node = &texture_node_root; 
			gd_get_frame(gif); texture_node = &(*texture_node)->next) 
	{
		*texture_node = malloc(sizeof(struct TextureNode));
		gd_render_frame(gif, frame);
		SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(frame, gif->width, gif->height, 24, gif->width * 3, 0x0000FF, 0x00FF00, 0xFF0000, 0);
		(*texture_node)->texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
		(*texture_node)->next = texture_node_root;
	}
	for (int running = 1; running;) {
		static SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type)
				case SDL_QUIT: running = 0; break;
		}
		SDL_RenderCopy(renderer, texture_node_root->texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_Delay(gif->gce.delay * 10 * 3 / 2);
		texture_node_root = texture_node_root->next;
	}
	ret_code = 0;
cleanup_texture_node_root:
	for (struct TextureNode** texture_node = &texture_node_root; *texture_node; *texture_node = NULL) {
		SDL_DestroyTexture((*texture_node)->texture);
		struct TextureNode** texture_node_next = &(*texture_node)->next;
		free(*texture_node);
		texture_node = texture_node_next;
	}
cleanup_frame:
	free(frame);
cleanup_window_renderer:
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
cleanup_sdl:
	SDL_Quit();
cleanup_gif:
	gd_close_gif(gif);

	return ret_code;
}
