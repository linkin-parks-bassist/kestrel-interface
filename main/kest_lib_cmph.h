#ifndef KEST_LIB_COMPILATION_HELPER_H_
#define KEST_LIB_COMPILATION_HELPER_H_

// Definitions, etc, needed for compilation to proceed, but not
// actually needed for the library to function. Better than having
// #ifdef KEST_LIBRARY\#endif everywhere in the code proper

#define PARAM_WIDGET_VIRTUAL_POT  0
#define PARAM_WIDGET_HSLIDER 	  1
#define PARAM_WIDGET_VSLIDER 	  2
#define PARAM_WIDGET_VSLIDER_TALL 3

#define SETTING_WIDGET_DROPDOWN	 0
#define SETTING_WIDGET_SWITCH 	 1
#define SETTING_WIDGET_FIELD 	 2

#define KEST_FPGA_SIMULATED

#endif
