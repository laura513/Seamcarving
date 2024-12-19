# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include "c_img.h"
 
double findSqrt(double x)
{
    // for 0 and 1, the square roots are themselves
    if (x < 2)
        return x;
 
    // considering the equation values
    double y = x;
    double z = (y + (x / y)) / 2;
 
    // as we want to get upto 5 decimal digits, the absolute
    // difference should not exceed 0.00001
    while (fabs(y - z) >= 0.00001) {
        y = z;
        z = (y + (x / y)) / 2;
    }
    return z;
}

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    int width = im->width;
    int height = im->height;
    create_img(grad, height, width);
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            uint8_t res;
            int Ry, Gy, By, delta_y, Rx, Gx, Bx, delta_x;
            if(y==0){
                Ry = (int)(get_pixel(im, y+1, x, 0) - get_pixel(im, height-1, x, 0));
                Gy = (int)(get_pixel(im, y+1, x, 1) - get_pixel(im, height-1, x, 1));
                By = (int)(get_pixel(im, y+1, x, 2) - get_pixel(im, height-1, x, 2));
                delta_y = Ry*Ry + Gy*Gy + By*By;
            }else if (y == height-1){
                Ry = (int)(get_pixel(im, 0, x, 0) - get_pixel(im, y-1, x, 0));
                Gy = (int)(get_pixel(im, 0, x, 1) - get_pixel(im, y-1, x, 1));
                By = (int)(get_pixel(im, 0, x, 2) - get_pixel(im, y-1, x, 2));
                delta_y = Ry*Ry + Gy*Gy + By*By;
            }else{
                Ry = (int)(get_pixel(im, y+1, x, 0) - get_pixel(im, y-1, x, 0));
                Gy = (int)(get_pixel(im, y+1, x, 1) - get_pixel(im, y-1, x, 1));
                By = (int)(get_pixel(im, y+1, x, 2) - get_pixel(im, y-1, x, 2));
                delta_y = Ry*Ry + Gy*Gy + By*By;
            }

            if (x == 0){
                Rx = (int)(get_pixel(im, y, x+1, 0) - get_pixel(im, y, width-1, 0));
                Gx = (int)(get_pixel(im, y, x+1, 1) - get_pixel(im, y, width-1, 1));
                Bx = (int)(get_pixel(im, y, x+1, 2) - get_pixel(im, y, width-1, 2));
                delta_x = Rx*Rx + Gx*Gx + Bx*Bx;
            }else if (x == width-1){
                Rx = (int)(get_pixel(im, y, 0, 0) - get_pixel(im, y, x-1, 0));
                Gx = (int)(get_pixel(im, y, 0, 1) - get_pixel(im, y, x-1, 1));
                Bx = (int)(get_pixel(im, y, 0, 2) - get_pixel(im, y, x-1, 2));
                delta_x = Rx*Rx + Gx*Gx + Bx*Bx;
            }else{
                Rx = (int)(get_pixel(im, y, x+1, 0) - get_pixel(im, y, x-1, 0));
                Gx = (int)(get_pixel(im, y, x+1, 1) - get_pixel(im, y, x-1, 1));
                Bx = (int)(get_pixel(im, y, x+1, 2) - get_pixel(im, y, x-1, 2));
                delta_x = Rx*Rx + Gx*Gx + Bx*Bx;
            }
            // double num;
            // // num = findSqrt((double)(delta_y + delta_x));

            // res = (uint8_t)(delta_y + delta_x);
            double num = findSqrt((double)(delta_y + delta_x));

            // Make sure the result is within the uint8_t range before casting.
            res = (uint8_t)(num/10);

            set_pixel(*grad, y, x, res, res, res);
        }
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr){
    int width = grad -> width;
    int height = grad -> height;
    *best_arr = (double *)malloc(width * height * sizeof(double));
    double self, top, side, left, right, min;
    double last_arr[width];
    for(int row = 0; row < height; row++){
        double cur_arr[width];
        for(int col = 0; col < width; col++){
            if(row == 0){
                (*best_arr)[row * width + col] = (double)(get_pixel(grad, row, col, 0));
                cur_arr[col] = (double)(get_pixel(grad, row, col, 0));
            }else if (col == 0){
                self = (double)(get_pixel(grad, row, col, 0));
                top = last_arr[col];
                side = last_arr[col + 1];
                min = (top < side) ? top : side;
                (*best_arr)[row * width + col] = self + min;
                cur_arr[col] = self + min;
            }else if (col == width-1){
                self = (double)(get_pixel(grad, row, col, 0));
                top = last_arr[col];
                side = last_arr[col - 1];
                min = (top < side) ? top : side;
                (*best_arr)[row * width + col] = self + min;
                cur_arr[col] = self + min;
            }else{
                self = (double)(get_pixel(grad, row, col, 0));
                top = last_arr[col];
                left = last_arr[col - 1];
                right = last_arr[col + 1];
                min = (left < right) ? ((left < top) ? left : top) : ((right < top) ? right : top);
                (*best_arr)[row * width + col] = self + min;
                cur_arr[col] = self + min;
            }
        }
        for(int i = 0; i < width; i++){
            last_arr[i] = cur_arr[i];
        }
    }   
}

int min_ind_of_int(double *paths, int size, int start){
    double min = paths[start];
    int ind = start; 
    for(int i = 1; i < size; i++){ 
        if(paths[start + i] < min){
            min = paths[start + i];
            ind = i; // Update to use the absolute index
        }
    }

    return ind;
}
void recover_path(double *best, int height, int width, int **path){
    *path = (int *)malloc(height * sizeof(int));
    for(int row = 0; row < height; row++){
        (*path)[row] = min_ind_of_int(best, width, row*width); 
    }
}
void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path) {
    int width = src->width;
    int height = src->height;
    create_img(dest, height, width - 1); 
    uint8_t R, G, B;

    for (int y = 0; y < height; y++) {
        int k = 0; // Reset k for each row
        for (int x = 0; x < width; x++) {
            if (x == path[y]) {
                continue; 
            } else {
                R = get_pixel(src, y, x, 0);
                G = get_pixel(src, y, x, 1);
                B = get_pixel(src, y, x, 2);
                set_pixel(*dest, y, k, R, G, B);
                k++; 
            }
        }
    }
}

// int main(void){
//     struct rgb_img *im;
//     read_in_img(&im, "6x5.bin");
//     struct rgb_img *grad;
//     calc_energy(im, &grad);
//     print_grad(grad);

//     int width = grad->width;
//     int height = grad->height;
//     double *best_arr;
//     dynamic_seam(grad, &best_arr);
//     for (int j = 0; j < height; j++) {
//         for (int i = 0; i < width; i++) {
//             printf("%.2f ", best_arr[j * width + i]);
//         }
//         printf("\n");
//     }

//     // int *path;
//     recover_path(best_arr, height, width, &path);
//     // for (int k = 0; k < height; k++){
//     //     printf("%d, ", path[k]);
//     // }

//     struct rgb_img *dest;
//     remove_seam(im, &dest, path);
//     print_grad(dest);

//     destroy_image(dest);
//     destroy_image(grad);
//     destroy_image(im);
// }