include $(TT_METAL_HOME)/libs/tensor/module.mk
include $(TT_METAL_HOME)/libs/dtx/module.mk
include $(TT_METAL_HOME)/libs/tt_dnn/module.mk
include $(TT_METAL_HOME)/libs/tt_lib/module.mk

LIBS_INCLUDES = $(COMMON_INCLUDES) -I$(TT_METAL_HOME)/libs/

TT_LIBS_TO_BUILD = libs/tensor \
                   libs/dtx \
                   libs/tt_dnn \
                   libs/tt_lib \

libs: $(TT_LIBS_TO_BUILD)
