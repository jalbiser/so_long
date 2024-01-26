#include "so_long.h"
#include "../MLX42/include/MLX42/MLX42.h"
#include <stdio.h>

typedef struct objects
{
    int type;
    int is_active;
    mlx_image_t* sprite;
    struct objects *next;
} t_object;



static int  create_map()
{
	
}

mlx_image_t* img;
t_object *wall;
t_object *objs = NULL;

int collision_check(int new_x, int new_y)
{
    t_object *current;
    current = objs;

    while (current != NULL)
    {
        if (!(new_x + img->width <= current->sprite->instances[0].x ||
              new_x >= current->sprite->instances[0].x + current->sprite->width ||
              new_y + img->height <= current->sprite->instances[0].y ||
              new_y >= current->sprite->instances[0].y + current->sprite->height) && current->is_active)
        {
            return 1; // Collision détectée
        }
        printf("Collision with object at (%d, %d)\n", current->sprite->instances[0].x, current->sprite->instances[0].y);
        current = current->next;
    }

    return 0; // Aucune collision détectée
}


int take_item(int new_x, int new_y)
{
    t_object *current;
    current = objs;

    while (current != NULL)
    {
        int img_center_x = new_x + img->width / 2;
        int img_center_y = new_y + img->height / 2;
        int obj_center_x = current->sprite->instances[0].x + current->sprite->width / 2;
        int obj_center_y = current->sprite->instances[0].y + current->sprite->height / 2;

        int combined_radius = current->sprite->width;

        int diff_x = img_center_x - obj_center_x;
        int diff_y = img_center_y - obj_center_y;

        if (current->type == 2 &&
            diff_x * diff_x < (current->sprite->width / 2 + combined_radius) * (current->sprite->width / 2 + combined_radius) &&
            diff_y * diff_y < (current->sprite->height / 2 + combined_radius) * (current->sprite->height / 2 + combined_radius))
        {
            current->sprite->enabled = false;
            current->is_active = 0;
            return 1;
        }
        current = current->next;
    }

    return 0;
}

#include <stdbool.h>
#include <math.h>

// Déclarer des variables globales pour suivre l'état du saut et du sol
bool is_jumping = false;
bool is_on_ground = true;

void hook(void *param)
{
    mlx_t *mlx;
    mlx = param;

    if (mlx_is_key_down(param, MLX_KEY_ESCAPE))
        mlx_close_window(param);

    int new_x = img->instances[0].x;
    int new_y = img->instances[0].y;

    static double jump_velocity = -15.0; // Vitesse initiale du saut (vers le haut)
    static double gravity = 1.0;          // Accélération gravitationnelle
    static double max_jump_height = 100.0; // Hauteur maximale du saut
    static double jump_timer = 0.0;        // Timer pour suivre le temps écoulé pendant le saut

    // Gravité
    if (!collision_check(new_x, new_y + 5)) {
        new_y += gravity * 6; // Appliquer la gravité
        is_on_ground = false; // Le personnage est en l'air
    } else {
        // Le personnage est au sol
        is_on_ground = true;
        is_jumping = false; // Réinitialiser l'état du saut
        jump_velocity = -15.0; // Réinitialiser la vitesse de saut
    }

    // Saut
    if (mlx_is_key_down(param, MLX_KEY_UP) && is_on_ground && !is_jumping)
    {
        is_jumping = true; // Déclencher le saut
        is_on_ground = false; // Le personnage n'est plus au sol
        jump_timer = 0.0;   // Réinitialiser le timer
    }

    if (is_jumping) {
        // Appliquer le mouvement vertical
        new_y += jump_velocity * 1.5;
        jump_velocity += gravity;

        // Vérifier si le personnage a atteint la hauteur maximale du saut
        if (jump_velocity >= 0.0 || fabs(new_y - img->instances[0].y) >= max_jump_height) {
            is_jumping = false;
            jump_velocity = -15.0; // Réinitialiser la vitesse de saut pour le prochain saut
        }
    }

    // Autres contrôles de mouvement
    if (mlx_is_key_down(param, MLX_KEY_DOWN) && !collision_check(new_x, new_y + 5))
    {
        new_y += 5;
        printf("Moving DOWN\n");
    }
    if (mlx_is_key_down(param, MLX_KEY_LEFT) && !collision_check(new_x - 5, new_y))
    {
        new_x -= 5;
        printf("Moving LEFT\n");
    }
    if (mlx_is_key_down(param, MLX_KEY_RIGHT) && !collision_check(new_x + 5, new_y))
    {
        new_x += 5;
        printf("Moving RIGHT\n");
    }
    if (mlx_is_key_down(param, MLX_KEY_F) && take_item(new_x, new_y))
    {
        printf("CAPTURE the item\n");
    }

    img->instances[0].x = new_x;
    img->instances[0].y = new_y;
}






void    create_object(int is_active, mlx_image_t* sprite, int type)
{
    t_object *new;
    t_object *current;

    new = malloc(sizeof(t_object));
    new->is_active = is_active;
    new->sprite = sprite;
    new->type = type;
    new->next = NULL;

    if (objs == NULL) {
        objs = new;
    } 
    else {
        current = objs;
        while (current->next != NULL)
            current = current->next;
        current->next = new;
    }
}

mlx_image_t *get_last_object()
{
    t_object *current;
    current = objs;
    while (current->next != NULL)
        current = current->next;
    return (current->sprite);
}

void	ft_lstclear(t_object *lst)
{
	t_object	*current;
	t_object	*next;

	if (!lst)
		return ;
	current = lst;
	while (current)
	{
		next = current->next;
		free(current);
		current = next;
	}
	lst = NULL;
}

void custom_memmove(uint8_t *dest, const uint8_t *src, size_t n) {
    size_t i = 0;
    while (i < n) {
        dest[i] = src[i];
        i++;
    }
}

mlx_image_t* cut_tiles(mlx_t* mlx, mlx_texture_t* texture, int line_x, int line_y, int width, int height, int size_x, int size_y) {
    int start_x = line_x * width;
    int start_y = line_y * height;
    if (!mlx || !texture || start_x < 0 || start_y < 0 || width <= 0 || height <= 0 ||
        start_x + width > texture->width || start_y + height > texture->height)
        return NULL;

    mlx_image_t* image = mlx_new_image(mlx, width, height);
    if (!image)
        return NULL;

    int i = 0;
    while (i < height) {
        int j = 0;
        while (j < width) {
            int k = 0;
            while (k < texture->bytes_per_pixel) {
                uint8_t* pixelx = &texture->pixels[(((i + start_y) * texture->width) + start_x + j) * texture->bytes_per_pixel + k];
                uint8_t* pixeli = &image->pixels[((i * width) + j) * texture->bytes_per_pixel + k];
                *pixeli = *pixelx;
                k++;
            }
            j++;
        }
        i++;
    }
    if (size_x != 0 || size_y != 0)
        mlx_resize_image(image, size_x, size_y);
    return image;
}

void display_map(mlx_t* mlx, t_map_info map)
{
    int a, b, z, p;
    a = 0;
    p = 0;
    while (a < map.y)
    {
        b = 0;
        z = 0;
        while (map.map[a][b])
        {
            if (map.map[a][b] == '1')
            {
                create_object(1, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 2, 2, 32, 32, 64, 64), 1);
                mlx_image_to_window(mlx, get_last_object(), z, p);
            }
            if (map.map[a][b] == '5')
            {
                if (b == 1 && a == 1)
                {
                    create_object(1, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 7, 1, 32, 32, 64, 64), 1);
                    mlx_image_to_window(mlx, get_last_object(), z, p);
                }
                else if ((b == map.x - 2) && a == 1)
                {
                    create_object(1, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 8, 1, 32, 32, 64, 64), 1);
                    mlx_image_to_window(mlx, get_last_object(), z, p);
                }
                else if (a == 1)
                {
                    create_object(1, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 2, 3, 32, 32, 64, 64), 1);
                    mlx_image_to_window(mlx, get_last_object(), z, p);
                }
                else if (b == 1 && a != map.y - 2)
                {
                    create_object(1, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 3, 2, 32, 32, 64, 64), 1);
                    mlx_image_to_window(mlx, get_last_object(), z, p);
                }
                else if (b == 1 && a == map.y - 2)
                {
                   create_object(1, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 7, 2, 32, 32, 64, 64), 1);
                    mlx_image_to_window(mlx, get_last_object(), z, p);
                }
                else if (a == map.y - 2 && (b != map.x - 2))
                {
                    create_object(1, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 2, 1, 32, 32, 64, 64), 1);
                    mlx_image_to_window(mlx, get_last_object(), z, p);
                }
                else if ((a == map.y - 2) && (b == map.x - 2))
                {
                    create_object(1, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 8, 2, 32, 32, 64, 64), 1);
                    mlx_image_to_window(mlx, get_last_object(), z, p);
                }
                else if (b == map.x - 2)
                {
                    create_object(1, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 1, 2, 32, 32, 64, 64), 1);
                    mlx_image_to_window(mlx, get_last_object(), z, p);
                }
            }
            if (map.map[a][b] == '0'  || map.map[a][b] == 'E' || map.map[a][b] == 'C'  || map.map[a][b] == 'P')
            {
                if (b == 1 && a == 2)
                    mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 1, 7, 32, 32, 64, 64), z, p);
                else if ((b == map.x - 3) && a == 2)
                    mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 3, 7, 32, 32, 64, 64), z, p);
                else if (a == 2)
                    mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 2, 7, 32, 32, 64, 64), z, p);
                else if (b == 2 && a != map.y - 3)
                    mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 1, 8, 32, 32, 64, 64), z, p);
                else if (b == 2 && a == map.y - 3)
                    mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 1, 9, 32, 32, 64, 64), z, p);
                else if (a == map.y - 3 && (b != map.x - 3))
                    mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 2, 9, 32, 32, 64, 64), z, p);
                else if ((a == map.y - 3) && (b == map.x - 3))
                    mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 3, 9, 32, 32, 64, 64), z, p);
                else if (b == map.x - 3)
                    mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 3, 8, 32, 32, 64, 64), z, p);
                else
                    mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./assets/terrain/tile.png"), 2, 8, 32, 32, 64, 64), z, p);
            }
            z += 64;
            b++;
        }
        p += 64;
        a++;
    }
}


int heart_frame = 0;
#include <time.h> 
void ui_auto_refresh(void *param)
{
    static clock_t start_time = 0;
    static const int FPS = 10; // 10 FPS
    static const int FRAME_DURATION = CLOCKS_PER_SEC / FPS;

    mlx_t *mlx = (mlx_t *)param;
    clock_t current_time = clock();

    if ((current_time - start_time) >= FRAME_DURATION) {
        start_time = current_time;
        mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./assets/UI/hearth.png"), heart_frame, 0, 18, 14, 72, 56), 105, 94);
        mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./assets/UI/hearth.png"), heart_frame, 0, 18, 14, 72, 56), 148, 94);
        mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./assets/UI/hearth.png"), heart_frame, 0, 18, 14, 72, 56), 192, 94);
        heart_frame = (heart_frame + 1) % 8;

    }
}

void    ui(mlx_t *mlx)
{
    mlx_loop_hook(mlx, &ui_auto_refresh, mlx);
    mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./assets/UI/live_bar.png"), 0, 0, 66, 34, 264, 128), 60, 58);
    
}

int main(void)
{
	t_map_info  map;

	map = get_array_map("./map.ber");
	mlx_t* mlx = mlx_init(64 * map.x, 64 * map.y, "Test", true);
	mlx_texture_t* texture = mlx_load_png("./player.png");
	img = mlx_texture_to_image(mlx, texture);

	mlx_loop_hook(mlx, &hook, mlx);
    display_map(mlx, map);

    ui(mlx);
    mlx_image_to_window(mlx, cut_tiles(mlx, mlx_load_png("./Chest.png"), 1, 1, 16, 16, 100, 100), 150, 150);
    mlx_resize_image(img, 50, 50);
    mlx_image_to_window(mlx, img, 160, 160);
	mlx_loop(mlx);

	mlx_delete_image(mlx, img);
    while (objs != NULL)
    {
        mlx_delete_image(mlx, objs->sprite);
        objs = objs->next;
    }
    ft_lstclear(objs);
	mlx_delete_texture(texture);
	mlx_terminate(mlx);
	return (0);
}