#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// doua structuri diferite, una pentru imagini grayscale
// una pentru imagini rgb
typedef struct {
	int width;
	int height;
	int max_value;
	unsigned char **data;
	char format[3];
} ppm_image;

typedef struct {
	int width;
	int height;
	int max_value;
	unsigned char ***data;
	char format[3];
} ppm_image_rgb;

// o structura pentru zona de selectie
typedef struct {
	int x1, y1, x2, y2;
} select_zone;

// functie pentru a calcula i-ul si j-ul dupa rotatie
void rotate_pixels(int *new_i, int *new_j, int angle, int i, int j,
				   int width, int height)
{
	switch (angle) {
	case 90:
	case -270:
		*new_i = j;
		*new_j = height - 1 - i;
		break;
	case -90:
	case 270:
		*new_i = width - 1 - j;
		*new_j = i;
		break;
	case 180:
	case -180:
		*new_i = height - 1 - i;
		*new_j = width - 1 - j;
		break;
	case 360:
	case -360:
		*new_i = i;
		*new_j = j;
		break;
	}
}

// functia de rotatie
// verificam zonele de selectie in functie de ce tip de imagine e
// aplicam rotatia pe img sau pe img_rgb (in functie de care e incarcata)
// daca rotatia e de +-90 / +-270, se schimba width-ul si height-ul
void rotate_img(ppm_image *img, select_zone *zone, int angle)
{
	int width = zone->x2 - zone->x1;
	int height = zone->y2 - zone->y1;
	int rotated_width, rotated_height;

	if (angle == 90 || angle == -90 || angle == 270 || angle == -270) {
		rotated_width = height;
		rotated_height = width;
	} else {
		rotated_width = width;
		rotated_height = height;
	}

	unsigned char **rotated = (unsigned char **)malloc(rotated_height
							   * sizeof(unsigned char *));
	for (int i = 0; i < rotated_height; i++)
		rotated[i] = (unsigned char *)malloc(rotated_width
					  * sizeof(unsigned char));

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int new_i, new_j;
			rotate_pixels(&new_i, &new_j, angle, i, j, width, height);
			rotated[new_i][new_j] = img->data[zone->y1 + i][zone->x1 + j];
		}
	}

	if (zone->x1 == 0 && zone->y1 == 0 && zone->x2 == img->width &&
		zone->y2 == img->height) {
		for (int i = 0; i < img->height; i++)
			free(img->data[i]);
		free(img->data);

		img->data = (unsigned char **)malloc(rotated_height
					* sizeof(unsigned char *));
		for (int i = 0; i < rotated_height; i++)
			img->data[i] = (unsigned char *)malloc(rotated_width
							* sizeof(unsigned char));

		img->width = rotated_width;
		img->height = rotated_height;
	}

	for (int i = 0; i < rotated_height; i++)
		for (int j = 0; j < rotated_width; j++)
			img->data[zone->y1 + i][zone->x1 + j] = rotated[i][j];

	for (int i = 0; i < rotated_height; i++)
		free(rotated[i]);
	free(rotated);
}

void rotate_img_rgb(ppm_image_rgb *img_rgb, select_zone *zone, int angle)
{
	int width = zone->x2 - zone->x1;
	int height = zone->y2 - zone->y1;
	int rotated_width, rotated_height;

	if (angle == 90 || angle == -90 || angle == 270 || angle == -270) {
		rotated_width = height;
		rotated_height = width;
	} else {
		rotated_width = width;
		rotated_height = height;
	}

	unsigned char ***rotated_rgb = (unsigned char ***)malloc(rotated_height
									* sizeof(unsigned char **));
	for (int i = 0; i < rotated_height; i++) {
		rotated_rgb[i] = (unsigned char **)malloc(rotated_width
						  * sizeof(unsigned char *));
		for (int j = 0; j < rotated_width; j++)
			rotated_rgb[i][j] = (unsigned char *)malloc(3
								* sizeof(unsigned char));
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int new_i, new_j;
			rotate_pixels(&new_i, &new_j, angle, i, j, width, height);
			for (int c = 0; c < 3; c++)
				rotated_rgb[new_i][new_j][c] = img_rgb->data[zone->y1 + i]
											   [zone->x1 + j][c];
		}
	}

	if (zone->x1 == 0 && zone->y1 == 0 && zone->x2 == img_rgb->width &&
		zone->y2 == img_rgb->height) {
		for (int i = 0; i < img_rgb->height; i++) {
			for (int j = 0; j < img_rgb->width; j++)
				free(img_rgb->data[i][j]);
			free(img_rgb->data[i]);
		}
		free(img_rgb->data);

		img_rgb->data = (unsigned char ***)malloc(rotated_height
						* sizeof(unsigned char **));
		for (int i = 0; i < rotated_height; i++) {
			img_rgb->data[i] = (unsigned char **)malloc(rotated_width
								* sizeof(unsigned char *));
			for (int j = 0; j < rotated_width; j++)
				img_rgb->data[i][j] = (unsigned char *)malloc(3
									  * sizeof(unsigned char));
		}

		img_rgb->width = rotated_width;
		img_rgb->height = rotated_height;
	}

	for (int i = 0; i < rotated_height; i++)
		for (int j = 0; j < rotated_width; j++)
			for (int c = 0; c < 3; c++)
				img_rgb->data[zone->y1 + i][zone->x1 + j][c] = rotated_rgb[i]
															   [j][c];

	for (int i = 0; i < rotated_height; i++) {
		for (int j = 0; j < rotated_width; j++)
			free(rotated_rgb[i][j]);
		free(rotated_rgb[i]);
	}
	free(rotated_rgb);
}

void rotate_command(ppm_image *img, ppm_image_rgb *img_rgb,
					select_zone *zone, int angle)
{
	if (!(angle == 90 || angle == -90 || angle == 180 || angle == -180 ||
		  angle == 270 || angle == -270 || angle == 360 ||
		  angle == -360 || angle == 0)) {
		printf("Unsupported rotation angle\n");
		return;
	}

	if (angle == 0) {
		printf("Rotated %d\n", angle);
		return;
	}

	if (img) {
		rotate_img(img, zone, angle);
	} else if (img_rgb) {
		rotate_img_rgb(img_rgb, zone, angle);
	}

	printf("Rotated %d\n", angle);
}

// functie pentru alocare de imagini grayscale
// adica care contin datele intr-un array 2d
void allocate_grayscale(ppm_image *image)
{
	image->data =
		(unsigned char **)malloc(image->height * sizeof(unsigned char *));
	for (int i = 0; i < image->height; i++) {
		image->data[i] =
			(unsigned char *)malloc(image->width * sizeof(unsigned char));
	}
}

// functie pentru alocare de imagini rgb
// adica care contin datele intr-un array 3d
void allocate_color(ppm_image_rgb *image)
{
	image->data =
		(unsigned char ***)malloc(image->height * sizeof(unsigned char **));
	for (int i = 0; i < image->height; i++) {
		image->data[i] =
			(unsigned char **)malloc(image->width * sizeof(unsigned char *));
		for (int j = 0; j < image->width; j++) {
			image->data[i][j] =
				(unsigned char *)malloc(3 * sizeof(unsigned char));
		}
	}
}

void free_grayscale(ppm_image *image)
{
	for (int i = 0; i < image->height; i++) {
		free(image->data[i]);
	}
	free(image->data);
}

void free_color(ppm_image_rgb *image)
{
	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			free(image->data[i][j]);
		}
		free(image->data[i]);
	}
	free(image->data);
}

// functia de load
// citim mai intai formatul, daca e de tipul celor grayscale
// atunci alocam o matrice grayscale (image)
// daca e rgb, alocam matrice rgb (image_rgb)
void load_command(const char *filename, ppm_image **image,
				  ppm_image_rgb **image_rgb)
{
	FILE *file = fopen(filename, "rb");
	if (!file) {
		printf("Failed to load %s\n", filename);
		return;
	}

	char format[3], temp_char[1];
	fscanf(file, "%2s", format);

	if (strcmp(format, "P1") == 0 || strcmp(format, "P2") == 0 ||
		strcmp(format, "P4") == 0 || strcmp(format, "P5") == 0) {
		if (!*image) {
			*image = (ppm_image *)malloc(sizeof(ppm_image));
		}
		strcpy((*image)->format, format);
		fscanf(file, "%d %d", &(*image)->width, &(*image)->height);

		if (strcmp(format, "P2") == 0 || strcmp(format, "P5") == 0) {
			fscanf(file, "%d", &(*image)->max_value);
			fscanf(file, "%c", temp_char);
		} else
			(*image)->max_value = 1;
		allocate_grayscale(*image);

		if (strcmp(format, "P2") == 0 || strcmp(format, "P1") == 0) {
			for (int i = 0; i < (*image)->height; i++) {
				for (int j = 0; j < (*image)->width; j++) {
					fscanf(file, "%hhu", &(*image)->data[i][j]);
				}
			}
		} else {
			for (int i = 0; i < (*image)->height; i++) {
				fread((*image)->data[i], sizeof(unsigned char), (*image)->width,
					  file);
			}
		}

	} else if (strcmp(format, "P3") == 0 || strcmp(format, "P6") == 0) {
		if (!*image_rgb) {
			*image_rgb = (ppm_image_rgb *)malloc(sizeof(ppm_image_rgb));
		}
		strcpy((*image_rgb)->format, format);
		fscanf(file, "%d %d %d", &(*image_rgb)->width, &(*image_rgb)->height,
			   &(*image_rgb)->max_value);

		allocate_color(*image_rgb);

		if (strcmp(format, "P3") == 0) {
			for (int i = 0; i < (*image_rgb)->height; i++) {
				for (int j = 0; j < (*image_rgb)->width; j++) {
					for (int k = 0; k < 3; k++) {
						fscanf(file, "%hhu", &(*image_rgb)->data[i][j][k]);
					}
				}
			}
		} else {
			fscanf(file, "%c", temp_char);
			for (int i = 0; i < (*image_rgb)->height; i++) {
				for (int j = 0; j < (*image_rgb)->width; j++) {
					fread((*image_rgb)->data[i][j], sizeof(unsigned char), 3,
						  file);
				}
			}
		}
	}

	printf("Loaded %s\n", filename);

	fclose(file);
}

// functia de sharpen
void sharpen(ppm_image_rgb *image_rgb, select_zone *selected_zone)
{
	if (!image_rgb || !image_rgb->data) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}

	int kernel[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};
	int x1 = selected_zone->x1, y1 = selected_zone->y1;
	int x2 = selected_zone->x2, y2 = selected_zone->y2;

	int width = x2 - x1;
	int height = y2 - y1;

	if (width <= 0 || height <= 0) {
		printf("APPLY parameter invalid\n");
		return;
	}
	int start_x = (x1 > 0) ? x1 : 1, start_y = (y1 > 0) ? y1 : 1;
	int end_x = (x2 < image_rgb->width) ? x2 : image_rgb->width - 1;
	int end_y = (y2 < image_rgb->height) ? y2 : image_rgb->height - 1;

	unsigned char ***temp_data =
		(unsigned char ***)malloc(image_rgb->height * sizeof(unsigned char **));
	for (int i = 0; i < image_rgb->height; i++) {
		temp_data[i] = (unsigned char **)malloc(image_rgb->width *
												sizeof(unsigned char *));
		for (int j = 0; j < image_rgb->width; j++) {
			temp_data[i][j] =
				(unsigned char *)malloc(3 * sizeof(unsigned char));
			for (int channel = 0; channel < 3; channel++) {
				temp_data[i][j][channel] = image_rgb->data[i][j][channel];
			}
		}
	}

	for (int i = start_y; i < end_y; i++) {
		for (int j = start_x; j < end_x; j++) {
			for (int channel = 0; channel < 3; channel++) {
				int pixel_value = 0;
				for (int ki = -1; ki <= 1; ki++) {
					for (int kj = -1; kj <= 1; kj++) {
						int ni = i + ki;
						int nj = j + kj;
						pixel_value += image_rgb->data[ni][nj][channel] *
									   kernel[ki + 1][kj + 1];
					}
				}

				pixel_value = pixel_value < 0
								  ? 0
								  : (pixel_value > 255 ? 255 : pixel_value);

				temp_data[i][j][channel] = (unsigned char)pixel_value;
			}
		}
	}

	for (int i = start_y; i < end_y; i++) {
		for (int j = start_x; j < end_x; j++) {
			for (int channel = 0; channel < 3; channel++) {
				image_rgb->data[i][j][channel] = temp_data[i][j][channel];
			}
		}
	}

	for (int i = 0; i < image_rgb->height; i++) {
		for (int j = 0; j < image_rgb->width; j++)
			free(temp_data[i][j]);
		free(temp_data[i]);
	}
	free(temp_data);

	printf("APPLY SHARPEN done\n");
}

// functia de edge
void edge(ppm_image_rgb *image_rgb, select_zone *selected_zone)
{
	if (!image_rgb || !image_rgb->data) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}
	int kernel[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
	int x1 = selected_zone->x1, y1 = selected_zone->y1;
	int x2 = selected_zone->x2, y2 = selected_zone->y2;
	int width = x2 - x1, height = y2 - y1;

	if (width <= 0 || height <= 0) {
		printf("APPLY parameter invalid\n");
		return;
	}

	int start_x = (x1 > 0) ? x1 : 1, start_y = (y1 > 0) ? y1 : 1;
	int end_x = (x2 < image_rgb->width) ? x2 : image_rgb->width - 1;
	int end_y = (y2 < image_rgb->height) ? y2 : image_rgb->height - 1;

	unsigned char ***temp_data =
		(unsigned char ***)malloc(image_rgb->height * sizeof(unsigned char **));
	for (int i = 0; i < image_rgb->height; i++) {
		temp_data[i] = (unsigned char **)malloc(image_rgb->width *
												sizeof(unsigned char *));
		for (int j = 0; j < image_rgb->width; j++) {
			temp_data[i][j] =
				(unsigned char *)malloc(3 * sizeof(unsigned char));
			for (int channel = 0; channel < 3; channel++)
				temp_data[i][j][channel] = image_rgb->data[i][j][channel];
		}
	}

	for (int i = start_y; i < end_y; i++)
		for (int j = start_x; j < end_x; j++)
			for (int channel = 0; channel < 3; channel++) {
				int pixel_value = 0;

				for (int ki = -1; ki <= 1; ki++)
					for (int kj = -1; kj <= 1; kj++) {
						int ni = i + ki;
						int nj = j + kj;

						pixel_value += image_rgb->data[ni][nj][channel] *
									   kernel[ki + 1][kj + 1];
					}
				pixel_value = pixel_value < 0
								  ? 0
								  : (pixel_value > 255 ? 255 : pixel_value);

				temp_data[i][j][channel] = (unsigned char)pixel_value;
			}

	for (int i = start_y; i < end_y; i++) {
		for (int j = start_x; j < end_x; j++) {
			for (int channel = 0; channel < 3; channel++) {
				image_rgb->data[i][j][channel] = temp_data[i][j][channel];
			}
		}
	}

	for (int i = 0; i < image_rgb->height; i++) {
		for (int j = 0; j < image_rgb->width; j++) {
			free(temp_data[i][j]);
		}
		free(temp_data[i]);
	}
	free(temp_data);

	printf("APPLY EDGE done\n");
}

// functie pentru aplicarea kernelului
// am folosit aceasta functie doar pentru blur/gaussian blur deoarece
// trecea functia blur de limita de linii
unsigned char apply_kernel(ppm_image_rgb *image, int i, int j,
						   int channel, double kernel[3][3])
{
	double pixel_value = 0.0;

	for (int ki = -1; ki <= 1; ki++) {
		for (int kj = -1; kj <= 1; kj++) {
			int ni = i + ki;
			int nj = j + kj;

			if (ni < 0 || ni >= image->height || nj < 0 || nj >= image->width)
				continue;

			pixel_value +=
				image->data[ni][nj][channel] * kernel[ki + 1][kj + 1];
		}
	}

	if (pixel_value < 0.0)
		pixel_value = 0.0;
	else if (pixel_value > 255.0)
		pixel_value = 255.0;

	return (unsigned char)pixel_value;
}

// functia de blur
void blur(ppm_image_rgb *image_rgb, select_zone *selected_zone)
{
	if (!image_rgb || !image_rgb->data) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}

	double kernel[3][3] = {{1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0},
						   {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0},
						   {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0}};

	int x1 = selected_zone->x1, y1 = selected_zone->y1;
	int x2 = selected_zone->x2, y2 = selected_zone->y2;
	int width = x2 - x1, height = y2 - y1;

	if (width <= 0 || height <= 0) {
		printf("APPLY parameter invalid\n");
		return;
	}

	int start_x = (x1 > 0) ? x1 : 1;
	int start_y = (y1 > 0) ? y1 : 1;
	int end_x = (x2 < image_rgb->width) ? x2 : image_rgb->width - 1;
	int end_y = (y2 < image_rgb->height) ? y2 : image_rgb->height - 1;

	unsigned char ***temp_data =
		malloc(image_rgb->height * sizeof(unsigned char **));
	for (int i = 0; i < image_rgb->height; i++) {
		temp_data[i] = malloc(image_rgb->width * sizeof(unsigned char *));
		for (int j = 0; j < image_rgb->width; j++) {
			temp_data[i][j] = malloc(3 * sizeof(unsigned char));
			for (int channel = 0; channel < 3; channel++)
				temp_data[i][j][channel] = image_rgb->data[i][j][channel];
		}
	}

	for (int i = start_y; i < end_y; i++) {
		for (int j = start_x; j < end_x; j++) {
			for (int channel = 0; channel < 3; channel++) {
				temp_data[i][j][channel] =
					apply_kernel(image_rgb, i, j, channel, kernel);
			}
		}
	}

	for (int i = start_y; i < end_y; i++)
		for (int j = start_x; j < end_x; j++)
			for (int channel = 0; channel < 3; channel++)
				image_rgb->data[i][j][channel] = temp_data[i][j][channel];

	for (int i = 0; i < image_rgb->height; i++) {
		for (int j = 0; j < image_rgb->width; j++)
			free(temp_data[i][j]);
		free(temp_data[i]);
	}
	free(temp_data);

	printf("APPLY BLUR done\n");
}

// functia de gaussian blur
void gaussian_blur(ppm_image_rgb *image_rgb, select_zone *selected_zone)
{
	if (!image_rgb || !image_rgb->data) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}

	double kernel[3][3] = {{1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0},
						   {2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0},
						   {1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0}};

	int x1 = selected_zone->x1, y1 = selected_zone->y1;
	int x2 = selected_zone->x2, y2 = selected_zone->y2;
	int width = x2 - x1, height = y2 - y1;

	if (width <= 0 || height <= 0) {
		printf("APPLY parameter invalid\n");
		return;
	}

	int start_x = (x1 > 0) ? x1 : 1;
	int start_y = (y1 > 0) ? y1 : 1;
	int end_x = (x2 < image_rgb->width) ? x2 : image_rgb->width - 1;
	int end_y = (y2 < image_rgb->height) ? y2 : image_rgb->height - 1;

	unsigned char ***temp_data =
		malloc(image_rgb->height * sizeof(unsigned char **));
	for (int i = 0; i < image_rgb->height; i++) {
		temp_data[i] = malloc(image_rgb->width * sizeof(unsigned char *));
		for (int j = 0; j < image_rgb->width; j++) {
			temp_data[i][j] = malloc(3 * sizeof(unsigned char));
			for (int channel = 0; channel < 3; channel++) {
				temp_data[i][j][channel] = image_rgb->data[i][j][channel];
			}
		}
	}

	for (int i = start_y; i < end_y; i++) {
		for (int j = start_x; j < end_x; j++) {
			for (int channel = 0; channel < 3; channel++) {
				temp_data[i][j][channel] =
					apply_kernel(image_rgb, i, j, channel, kernel);
			}
		}
	}

	for (int i = start_y; i < end_y; i++) {
		for (int j = start_x; j < end_x; j++) {
			for (int channel = 0; channel < 3; channel++) {
				image_rgb->data[i][j][channel] = temp_data[i][j][channel];
			}
		}
	}

	for (int i = 0; i < image_rgb->height; i++) {
		for (int j = 0; j < image_rgb->width; j++) {
			free(temp_data[i][j]);
		}
		free(temp_data[i]);
	}
	free(temp_data);

	printf("APPLY GAUSSIAN_BLUR done\n");
}

// functia de selectare a zonei
void select_command(ppm_image *image, ppm_image_rgb *image_rgb,
					select_zone *selected_zone, int x1, int y1, int x2,
					int y2)
{
	int aux;

	if (x1 > x2) {
		aux = x1;
		x1 = x2;
		x2 = aux;
	}

	if (y1 > y2) {
		aux = y1;
		y1 = y2;
		y2 = aux;
	}

	if (image) {
		if (x1 < 0 || y1 < 0 || x2 > image->width || y2 > image->height ||
			x1 == x2 || y1 == y2) {
			printf("Invalid set of coordinates\n");
			return;
		}
	}

	else if (image_rgb) {
		if (x1 < 0 || y1 < 0 || x2 > image_rgb->width ||
			y2 > image_rgb->height || x1 == x2 || y1 == y2) {
			printf("Invalid set of coordinates\n");
			return;
		}
	}

	selected_zone->x1 = x1;
	selected_zone->y1 = y1;
	selected_zone->x2 = x2;
	selected_zone->y2 = y2;

	printf("Selected %d %d %d %d\n", x1, y1, x2, y2);
}

void free_all(ppm_image **image, ppm_image_rgb **image_rgb)
{
	if (*image) {
		free_grayscale(*image);
		free(*image);
		*image = NULL;
	}
	if (*image_rgb) {
		free_color(*image_rgb);
		free(*image_rgb);
		*image_rgb = NULL;
	}
}

int check_loaded_file(ppm_image *image, ppm_image_rgb *image_rgb)
{
	if (!image && !image_rgb) {
		printf("No image loaded\n");
		return 0;
	}
	return 1;
}

// functia de save
// daca se salveaza ca ascii, dar formatul e binar atunci transfera formatul
// iar daca se salveaza ca binar se face invers
void save_command(ppm_image *image, ppm_image_rgb *image_rgb,
				  const char *filename, int is_ascii)
{
	FILE *file = fopen(filename, is_ascii ? "w" : "wb");

	if (image) {
		if (is_ascii) {
			if (strcmp(image->format, "P4") == 0)
				strcpy(image->format, "P1");
			else if (strcmp(image->format, "P5") == 0)
				strcpy(image->format, "P2");
		} else {
			if (strcmp(image->format, "P1") == 0)
				strcpy(image->format, "P4");
			else if (strcmp(image->format, "P2") == 0)
				strcpy(image->format, "P5");
		}

		fprintf(file, "%s\n%d %d\n", image->format, image->width,
				image->height);
		if (strcmp(image->format, "P2") == 0)
			fprintf(file, "%d\n",
					image->max_value);
		else if (strcmp(image->format, "P5") == 0)
			fprintf(file, "%d\n", image->max_value);

		if (is_ascii) {
			for (int i = 0; i < image->height; i++) {
				for (int j = 0; j < image->width; j++) {
					fprintf(file, "%d ", image->data[i][j]);
				}
				fprintf(file, "\n");
			}
		} else {
			for (int i = 0; i < image->height; i++) {
				fwrite(image->data[i], sizeof(unsigned char), image->width,
					   file);
			}
		}
	} else if (image_rgb) {
		if (is_ascii)
			strcpy(image_rgb->format, "P3");
		else
			strcpy(image_rgb->format, "P6");

		if (strcmp(image_rgb->format, "P3") == 0)
			fprintf(file, "%s\n%d %d\n%d\n", image_rgb->format,
					image_rgb->width, image_rgb->height, image_rgb->max_value);
		else
			fprintf(file, "%s\n%d %d\n%d\n", image_rgb->format,
					image_rgb->width, image_rgb->height, image_rgb->max_value);

		if (is_ascii) {
			for (int i = 0; i < image_rgb->height; i++) {
				for (int j = 0; j < image_rgb->width; j++) {
					for (int m = 0; m < 3; m++) {
						fprintf(file, "%d ", image_rgb->data[i][j][m]);
					}
				}
				fprintf(file, "\n");
			}
		} else {
			for (int i = 0; i < image_rgb->height; i++) {
				for (int j = 0; j < image_rgb->width; j++) {
					fwrite(image_rgb->data[i][j], sizeof(unsigned char), 3,
						   file);
				}
			}
		}
	}

	fclose(file);
	printf("Saved %s\n", filename);
}

// functia de crop
// utilizeaza o matrice auxiliara, o umple cu elemente iar apoi
// realoca matricea initiala, copiaza valorile si elibereaza matricea aux
void crop_command(ppm_image **image, ppm_image_rgb **image_rgb,
				  select_zone *selected_zone)
{
	int x1 = selected_zone->x1;
	int y1 = selected_zone->y1;
	int x2 = selected_zone->x2;
	int y2 = selected_zone->y2;

	int cropped_width = x2 - x1;
	int cropped_height = y2 - y1;

	if (x1 < 0 || y1 < 0 ||
		x2 > (*image ? (*image)->width : (*image_rgb)->width) ||
		y2 > (*image ? (*image)->height : (*image_rgb)->height)) {
		printf("Image cropped\n");
		return;
	}

	if (*image) {
		ppm_image *cropped_image = (ppm_image *)malloc(sizeof(ppm_image));
		cropped_image->width = cropped_width;
		cropped_image->height = cropped_height;
		strcpy(cropped_image->format, (*image)->format);
		cropped_image->max_value = (*image)->max_value;

		allocate_grayscale(cropped_image);

		for (int i = 0; i < cropped_height; i++) {
			for (int j = 0; j < cropped_width; j++) {
				cropped_image->data[i][j] = (*image)->data[y1 + i][x1 + j];
			}
		}

		free_grayscale(*image);
		free(*image);

		*image = cropped_image;

		printf("Image cropped\n");
	} else if (*image_rgb) {
		ppm_image_rgb *cropped_image_rgb =
			(ppm_image_rgb *)malloc(sizeof(ppm_image_rgb));
		cropped_image_rgb->width = cropped_width;
		cropped_image_rgb->height = cropped_height;
		strcpy(cropped_image_rgb->format, (*image_rgb)->format);
		cropped_image_rgb->max_value = (*image_rgb)->max_value;

		allocate_color(cropped_image_rgb);

		for (int i = 0; i < cropped_height; i++) {
			for (int j = 0; j < cropped_width; j++) {
				for (int m = 0; m < 3; m++) {
					cropped_image_rgb->data[i][j][m] =
						(*image_rgb)->data[y1 + i][x1 + j][m];
				}
			}
		}

		free_color(*image_rgb);
		free(*image_rgb);

		*image_rgb = cropped_image_rgb;

		printf("Image cropped\n");
	}
}

// functia de apply
void apply_command(ppm_image *image, ppm_image_rgb *image_rgb, char *parameter,
				   select_zone *selected_zone)
{
	char command[101];

	if (fgets(command, sizeof(command), stdin)) {
		command[strcspn(command, "\n")] = '\0';

		strncpy(parameter, command, 100);
		parameter[100] = '\0';
	}

	if (!check_loaded_file(image, image_rgb))
		return;
	if (strcmp(parameter, " SHARPEN") == 0)
		sharpen(image_rgb, selected_zone);
	else if (strcmp(parameter, " EDGE") == 0)
		edge(image_rgb, selected_zone);
	else if (strcmp(parameter, " BLUR") == 0)
		blur(image_rgb, selected_zone);
	else if (strcmp(parameter, " GAUSSIAN_BLUR") == 0)
		gaussian_blur(image_rgb, selected_zone);
	else if (parameter[0] == '\0' ||
			 strspn(parameter, " \t") == strlen(parameter))
		printf("Invalid command\n");
	else
		printf("APPLY parameter invalid\n");
}

// functie de verificare daca fisierul se salveaza ca ascii sau nu
void check_ascii(ppm_image *image, ppm_image_rgb *image_rgb)
{
	char read_line[101], first_word[101], ascii[101];
	if (check_loaded_file(image, image_rgb)) {
		fgets(read_line, sizeof(read_line), stdin);
		int cnt_words = sscanf(read_line, "%s %s", first_word, ascii);
		if (cnt_words == 1)
			save_command(image, image_rgb, first_word, 0);
		else
			save_command(image, image_rgb, first_word, 1);
	}

	else
		fgets(read_line, sizeof(read_line), stdin);
}

void wrong_command(void)
{
	char read_line[101];
	fgets(read_line, sizeof(read_line), stdin);
}

int check_grayscale(ppm_image *image, ppm_image_rgb *image_rgb)
{
	if (image) {
		if (strcmp(image->format, "P2") == 0 ||
			strcmp(image->format, "P5") == 0) {
			return 1;
		}
		printf("Black and white image needed\n");
		return 0;
	}

	if (image_rgb) {
		printf("Black and white image needed\n");
		return 0;
	}

	return 0;
}

// functia de egalizare
// utilizeaza doi vectori auxiliari, pentru calcularea histogramei
void equalize_command(ppm_image *image, ppm_image_rgb *image_rgb)
{
	if (check_grayscale(image, image_rgb)) {
		int width = image->width;
		int height = image->height;
		int area = width * height;

		int histogram[256] = {0};
		int min_val = 255, max_val = 0;

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				int pixel_value = image->data[i][j];
				histogram[pixel_value]++;
				if (pixel_value < min_val)
					min_val = pixel_value;
				if (pixel_value > max_val)
					max_val = pixel_value;
			}
		}

		int cumulative_histogram[256] = {0};
		cumulative_histogram[min_val] = histogram[min_val];
		for (int i = min_val + 1; i <= max_val; i++) {
			cumulative_histogram[i] =
				cumulative_histogram[i - 1] + histogram[i];
		}

		unsigned char mapping[256];
		for (int i = 0; i < 256; i++) {
			if (i < min_val) {
				mapping[i] = 0;
			} else if (i > max_val) {
				mapping[i] = 255;
			} else {
				mapping[i] = (unsigned char)round
				(255.0 * (double)cumulative_histogram[i] / area);
			}
		}

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				image->data[i][j] = mapping[image->data[i][j]];
			}
		}

		printf("Equalize done\n");
	}
}

// functia de histogram (nu e gata)
void histogram_command(ppm_image *image, ppm_image_rgb *image_rgb)
{
	int stars, bins;
	if (check_grayscale(image, image_rgb)) {
		scanf("%d %d", &stars, &bins);
	} else {
		scanf("%d %d", &stars, &bins);
	}
}

int main(void)
{
	char command[101], filename[101], apply_param[101];
	ppm_image *image = NULL;
	ppm_image_rgb *image_rgb = NULL;
	select_zone selected_zone = {0, 0, 0, 0};
	int x1, x2, y1, y2, angle;

	while (scanf("%s", command) == 1) {
		if (strcmp(command, "LOAD") == 0) {
			scanf("%s", filename);
			free_all(&image, &image_rgb);
			load_command(filename, &image, &image_rgb);
		} else if (strcmp(command, "SELECT") == 0) {
			char input[101];
			scanf("%s", input);
			if (check_loaded_file(image, image_rgb)) {
				if (strcmp(input, "ALL") == 0) {
					if (image) {
						selected_zone.x1 = 0;
						selected_zone.y1 = 0;
						selected_zone.x2 = image->width;
						selected_zone.y2 = image->height;
					} else if (image_rgb) {
						selected_zone.x1 = 0;
						selected_zone.y1 = 0;
						selected_zone.x2 = image_rgb->width;
						selected_zone.y2 = image_rgb->height;
					}
					printf("Selected ALL\n");
				} else {
					char line[256];
					x1 = atoi(input);
					getchar();
					fgets(line, sizeof(line), stdin);
					if (sscanf(line, "%d %d %d", &y1, &x2, &y2) == 3) {
						select_command(image, image_rgb, &selected_zone, x1,
									   y1, x2, y2);

						}
					else
						printf("Invalid command\n");
				}
			} else {
				if (strcmp(input, "ALL") != 0) {
					x1 = atoi(input);
					scanf("%d %d %d", &y1, &x2, &y2);
				}
			}
		} else if (strcmp(command, "SAVE") == 0)
			check_ascii(image, image_rgb);
		else if (strcmp(command, "APPLY") == 0)
			apply_command(image, image_rgb, apply_param, &selected_zone);
		else if (strcmp(command, "CROP") == 0) {
			if (check_loaded_file(image, image_rgb))
				crop_command(&image, &image_rgb, &selected_zone);
		} else if (strcmp(command, "EQUALIZE") == 0) {
			if (check_loaded_file(image, image_rgb))
				equalize_command(image, image_rgb);
		} else if (strcmp(command, "HISTOGRAM") == 0) {
			if (check_loaded_file(image, image_rgb))
				histogram_command(image, image_rgb);
		} else if (strcmp(command, "ROTATE") == 0) {
			if (check_loaded_file(image, image_rgb)) {
				scanf("%d", &angle);
				rotate_command(image, image_rgb, &selected_zone, angle);
			} else
				wrong_command();
		} else if (strcmp(command, "EXIT") == 0) {
			check_loaded_file(image, image_rgb);
			free_all(&image, &image_rgb);
			exit(0);
		} else {
			printf("Invalid command\n");
			wrong_command();
		}
	}
	return 0;
}
