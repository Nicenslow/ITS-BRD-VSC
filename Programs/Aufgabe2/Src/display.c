/**
 * @file display.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief Feste Beschriftung einmalig, Werte zeichenweise.
 */

#include "display.h"

#include "gpio_out.h"
#include "lcd.h"

#include <stdio.h>

#define DISPLAY_FONT_POINTS 12
#define LABEL_COLUMN ((POINT)1)
#define VALUE_COLUMN ((POINT)20)
#define ANGLE_ROW ((POINT)3)
#define VELOCITY_ROW ((POINT)5)
#define VALUE_WIDTH 16

typedef struct {
   char text[VALUE_WIDTH + 1];
   int length;
   int next_index;
   POINT row;
} OutputLine;

static OutputLine output_lines[2];
static int active_line = 2;

static void format_line(OutputLine *line, POINT row, double value) {
   int written = snprintf(line->text, sizeof(line->text), "%*.1f", VALUE_WIDTH - 1, value);
   if (written < 0) {
      line->text[0] = '\0';
      line->length = 0;
   } else if (written >= (int)sizeof(line->text)) {
      line->length = VALUE_WIDTH;
   } else {
      line->length = written;
   }
   line->next_index = 0;
   line->row = row;
}

void display_init(void) {
   lcdSetFont(DISPLAY_FONT_POINTS);
   lcdGotoXY(LABEL_COLUMN, ANGLE_ROW);
   lcdPrintS("Winkel (Grad):");
   lcdGotoXY(LABEL_COLUMN, VELOCITY_ROW);
   lcdPrintS("Geschw. (Grad/s):");
   active_line = 2;
}

void display_submit(double angle_deg, double velocity_deg_per_sec) {
   format_line(&output_lines[0], ANGLE_ROW, angle_deg);
   format_line(&output_lines[1], VELOCITY_ROW, velocity_deg_per_sec);
   active_line = 0;
}

bool display_is_busy(void) {
   return active_line < 2;
}

void display_pump(void) {
   OutputLine *line;

   if (active_line >= 2) {
      return;
   }

   line = &output_lines[active_line];
   if (line->next_index >= line->length) {
      active_line++;
      return;
   }

   if (line->next_index == 0) {
      lcdGotoXY(VALUE_COLUMN, line->row);
   }

   gpio_out_set_measure_pin(true);
   lcdPrintC(line->text[line->next_index]);
   gpio_out_set_measure_pin(false);
   line->next_index++;

   if (line->next_index >= line->length) {
      active_line++;
   }
}
