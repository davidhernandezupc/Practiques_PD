#include "spo2_algorithm.h"
#include <math.h>

#define true 1
#define false 0

static int32_t an_dx[100];
static int32_t an_x[100];

// Finds peaks above a threshold
static void maxim_find_peaks(int32_t *pn_locs, int32_t *pn_npks, int32_t *pn_x, int32_t n_size, int32_t n_min_height, int32_t n_min_distance, int32_t n_max_num) {
  int32_t i = 1, n_width;
  *pn_npks = 0;

  while (i < n_size - 1) {
    if (pn_x[i] > n_min_height && pn_x[i] > pn_x[i - 1]) {
      n_width = 1;
      while (i + n_width < n_size && pn_x[i] == pn_x[i + n_width])
        n_width++;
      if (pn_x[i] > pn_x[i + n_width] && (*pn_npks) < n_max_num) {
        pn_locs[*pn_npks] = i;
        (*pn_npks)++;
        i += n_width + 1;
      } else {
        i += n_width;
      }
    } else {
      i++;
    }
  }

  // Enforce minimum distance between peaks
  for (i = 1; i < *pn_npks; i++) {
    if (pn_locs[i] - pn_locs[i - 1] < n_min_distance) {
      if (pn_x[pn_locs[i - 1]] > pn_x[pn_locs[i]])
        pn_locs[i] = -1;
      else
        pn_locs[i - 1] = -1;
    }
  }

  // Clean up
  int32_t n_temp[15], j = 0;
  for (i = 0; i < *pn_npks; i++) {
    if (pn_locs[i] != -1) {
      n_temp[j++] = pn_locs[i];
    }
  }
  for (i = 0; i < j; i++) {
    pn_locs[i] = n_temp[i];
  }
  *pn_npks = j;
}

// Applies a moving average filter
static void maxim_moving_average(int32_t *pn_x, int32_t *pn_y, int32_t n_size, int32_t n_width) {
  int32_t i, j, n_sum;
  for (i = 0; i < n_size; i++) {
    n_sum = 0;
    for (j = 0; j < n_width; j++) {
      if ((i + j) < n_size)
        n_sum += pn_x[i + j];
      else
        n_sum += pn_x[n_size - 1];
    }
    pn_y[i] = n_sum / n_width;
  }
}

// Calculates HR and SpO2
void maxim_heart_rate_and_oxygen_saturation(uint32_t *pun_ir_buffer, int32_t n_ir_buffer_length,
                                            uint32_t *pun_red_buffer, int32_t *pn_spo2, int8_t *pch_spo2_valid,
                                            int32_t *pn_heart_rate, int8_t *pch_hr_valid) {
  int32_t i, n_th1, n_npks;
  int32_t an_ir_valley_locs[15];
  int32_t n_peak_interval_sum;

  // Remove DC and invert signal
  for (i = 0; i < n_ir_buffer_length; i++) {
    an_x[i] = (int32_t)pun_ir_buffer[i];
  }
  maxim_moving_average(an_x, an_dx, n_ir_buffer_length, 4);

  for (i = 0; i < n_ir_buffer_length; i++) {
    an_dx[i] = -an_dx[i]; // Invert signal
  }

  n_th1 = 30; // Peak detection threshold
  maxim_find_peaks(an_ir_valley_locs, &n_npks, an_dx, n_ir_buffer_length, n_th1, 4, 15);

  if (n_npks >= 2) {
    n_peak_interval_sum = 0;
    for (i = 1; i < n_npks; i++) {
      n_peak_interval_sum += (an_ir_valley_locs[i] - an_ir_valley_locs[i - 1]);
    }
    n_peak_interval_sum = n_peak_interval_sum / (n_npks - 1);
    *pn_heart_rate = (int32_t)(100 * 25 / n_peak_interval_sum);
    *pch_hr_valid = true;
  } else {
    *pn_heart_rate = -999;
    *pch_hr_valid = false;
  }

  // Calculate SpO2
  int32_t n_spo2;
  int32_t ir_ac, red_ac;
  int32_t ir_mean, red_mean;
  long nume, deno;
  float ratio;

  red_ac = red_mean = ir_ac = ir_mean = 0;
  for (i = 0; i < n_ir_buffer_length; i++) {
    red_mean += pun_red_buffer[i];
    ir_mean += pun_ir_buffer[i];
  }
  red_mean /= n_ir_buffer_length;
  ir_mean /= n_ir_buffer_length;

  for (i = 0; i < n_ir_buffer_length; i++) {
    red_ac += abs((int32_t)pun_red_buffer[i] - red_mean);
    ir_ac += abs((int32_t)pun_ir_buffer[i] - ir_mean);
  }

  if (ir_ac != 0 && red_ac != 0) {
    ratio = (float)red_ac / (float)ir_ac;
    n_spo2 = 110 - 25 * ratio;
    if (n_spo2 > 100) n_spo2 = 100;
    if (n_spo2 < 0) n_spo2 = 0;
    *pn_spo2 = n_spo2;
    *pch_spo2_valid = true;
  } else {
    *pn_spo2 = -999;
    *pch_spo2_valid = false;
  }
}