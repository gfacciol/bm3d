#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <string.h>

#include "bm3d.h"
#include "utilities.h"

#define YUV       0
#define YCBCR     1
#define OPP       2
#define RGB       3
#define DCT       4
#define BIOR      5
#define HADAMARD  6
#define NONE      7

using namespace std;

// c: pointer to original argc
// v: pointer to original argv
// o: option name after hyphen
// d: default value (if NULL, the option takes no argument)
const char *pick_option(int *c, char **v, const char *o, const char *d) {
  int id = d ? 1 : 0;
  for (int i = 0; i < *c - id; i++) {
    if (v[i][0] == '-' && 0 == strcmp(v[i] + 1, o)) {
      char *r = v[i + id] + 1 - id;
      for (int j = i; j < *c - id; j++)
        v[j] = v[j + id + 1];
      *c -= id + 1;
      return r;
    }
  }
  return d;
}

/**
 * @file   main.cpp
 * @brief  Main executable file. Do not use lib_fftw to
 *         process DCT.
 *
 * @author MARC LEBRUN  <marc.lebrun@cmla.ens-cachan.fr>
 */
int main(int argc, char **argv)
{
  //! Variables initialization
  const char *_tau_2D_hard = pick_option(&argc, argv, "tau_2d_hard", "bior");
  const char *_tau_2D_wien = pick_option(&argc, argv, "tau_2d_wien", "dct");
  const char *_color_space = pick_option(&argc, argv, "color_space", "opp");
  const bool useSD_1 = pick_option(&argc, argv, "useSD_hard", NULL) != NULL;
  const bool useSD_2 = pick_option(&argc, argv, "useSD_wien", NULL) != NULL;

	//! Check parameters
	const unsigned tau_2D_hard  = (strcmp(_tau_2D_hard, "dct" ) == 0 ? DCT :
                                 (strcmp(_tau_2D_hard, "bior") == 0 ? BIOR : NONE));
    if (tau_2D_hard == NONE) {
        cout << "tau_2d_hard is not known." << endl;
        argc = 0; //abort
    }
	const unsigned tau_2D_wien  = (strcmp(_tau_2D_wien, "dct" ) == 0 ? DCT :
                                 (strcmp(_tau_2D_wien, "bior") == 0 ? BIOR : NONE));
    if (tau_2D_wien == NONE) {
        cout << "tau_2d_wien is not known." << endl;
        argc = 0; //abort
    };
	const unsigned color_space  = (strcmp(_color_space, "rgb"  ) == 0 ? RGB   :
                                 (strcmp(_color_space, "yuv"  ) == 0 ? YUV   :
                                 (strcmp(_color_space, "ycbcr") == 0 ? YCBCR :
                                 (strcmp(_color_space, "opp"  ) == 0 ? OPP   : NONE))));
    if (color_space == NONE) {
        cout << "color_space is not known." << endl;
        argc = 0; //abort
    };


  //! Check if there is the right call for the algorithm
  if (argc < 4) {
    cerr << "usage: " << argv[0] << " input sigma output [basic]\n\
             [-tau_2d_hard {dct,bior} (default: bior)]\n\
             [-useSD_hard  \n\
             [-tau_2d_wien {dct,bior} (default: dct)]\n\
             [-useSD_wien  \n\
             [-color_space {rgb,yuv,opp,ycbcr} (default: opp)]" << endl;
    return EXIT_FAILURE;
  }

	//! Declarations
	vector<float> img_noisy, img_basic, img_denoised;
    unsigned width, height, chnls;

    //! Load image
	if(load_image(argv[1], img_noisy, &width, &height, &chnls) != EXIT_SUCCESS)
        return EXIT_FAILURE;

	float          fSigma       = atof(argv[2]);

    //! Denoising
    if (run_bm3d(fSigma, img_noisy, img_basic, img_denoised, width, height, chnls,
                 useSD_1, useSD_2, tau_2D_hard, tau_2D_wien, color_space)
        != EXIT_SUCCESS)
        return EXIT_FAILURE;

	//! save noisy, denoised and differences images
	cout << endl << "Save images...";

   if (argc > 4)
   if (save_image(argv[4], img_basic, width, height, chnls) != EXIT_SUCCESS)
      return EXIT_FAILURE;

	if (save_image(argv[3], img_denoised, width, height, chnls) != EXIT_SUCCESS)
		return EXIT_FAILURE;

    cout << "done." << endl;

	return EXIT_SUCCESS;
}
