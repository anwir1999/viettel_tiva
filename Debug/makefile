################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./AD9833.obj" \
"./ADS1241.obj" \
"./DS3231.obj" \
"./DigitalFilter_Lib.obj" \
"./Viettel_Tiva123_BlackBox.obj" \
"./tm4c123gh6pm_startup_ccs.obj" \
"./CAM/CAM.obj" \
"./Cellular/cellular.obj" \
"./MQTT/MQTT_HwLayer.obj" \
"./NetworkModule/AT_Function.obj" \
"./NetworkModule/GNSS.obj" \
"./NetworkModule/GPS.obj" \
"./NetworkModule/uartstdio.obj" \
"./RFID/RFID.obj" \
"./SD/SD.obj" \
"./TFT_LCD_Lib/ColorTFTSymbols.obj" \
"./TFT_LCD_Lib/IFTSPI2_2LCD.obj" \
"./third_party/fatfs/port/mmc-dk-tm4c123g.obj" \
"./third_party/fatfs/src/ff.obj" \
"./third_party/fatfs/src/option/cc932.obj" \
"./utils/cmdline.obj" \
"C:/ti/TivaWare_C_Series-2.1.3.156/driverlib/ccs/Debug/driverlib.lib" \
"C:/ti/TivaWare_C_Series-2.1.3.156/grlib/ccs/Debug/grlib.lib" \
"../tm4c123gh6pm.cmd" \
$(GEN_CMDS__FLAG) \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include CAM/subdir_vars.mk
-include Cellular/subdir_vars.mk
-include MQTT/subdir_vars.mk
-include NetworkModule/subdir_vars.mk
-include RFID/subdir_vars.mk
-include SD/subdir_vars.mk
-include TFT_LCD_Lib/subdir_vars.mk
-include third_party/fatfs/port/subdir_vars.mk
-include third_party/fatfs/src/subdir_vars.mk
-include third_party/fatfs/src/option/subdir_vars.mk
-include utils/subdir_vars.mk
-include subdir_rules.mk
-include CAM/subdir_rules.mk
-include Cellular/subdir_rules.mk
-include MQTT/subdir_rules.mk
-include NetworkModule/subdir_rules.mk
-include RFID/subdir_rules.mk
-include SD/subdir_rules.mk
-include TFT_LCD_Lib/subdir_rules.mk
-include third_party/fatfs/port/subdir_rules.mk
-include third_party/fatfs/src/subdir_rules.mk
-include third_party/fatfs/src/option/subdir_rules.mk
-include utils/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
Viettel_Tiva123_BlackBox.out \

EXE_OUTPUTS__QUOTED += \
"Viettel_Tiva123_BlackBox.out" \

BIN_OUTPUTS += \
Viettel_Tiva123_BlackBox.hex \

BIN_OUTPUTS__QUOTED += \
"Viettel_Tiva123_BlackBox.hex" \


# All Target
all: Viettel_Tiva123_BlackBox.out

# Tool invocations
Viettel_Tiva123_BlackBox.out: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Linker'
	"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -g --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RA1 --define=PART_TM4C123GH6PM --diag_wrap=off --diag_warning=225 --display_error_number --abi=eabi -z -m"Viettel_Tiva123_BlackBox.map" --stack_size=4096 --heap_size=4096 -i"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/lib" -i"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --reread_libs --diag_wrap=off --warn_sections --display_error_number --xml_link_info="Viettel_Tiva123_BlackBox_linkInfo.xml" --rom_model -o "Viettel_Tiva123_BlackBox.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

Viettel_Tiva123_BlackBox.hex: $(EXE_OUTPUTS)
	@echo 'Invoking: ARM Hex Utility'
	"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armhex"  -o "Viettel_Tiva123_BlackBox.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)$(BIN_OUTPUTS__QUOTED)
	-$(RM) "AD9833.d" "ADS1241.d" "DS3231.d" "DigitalFilter_Lib.d" "Viettel_Tiva123_BlackBox.d" "tm4c123gh6pm_startup_ccs.d" "CAM\CAM.d" "Cellular\cellular.d" "MQTT\MQTT_HwLayer.d" "NetworkModule\AT_Function.d" "NetworkModule\GNSS.d" "NetworkModule\GPS.d" "NetworkModule\uartstdio.d" "RFID\RFID.d" "SD\SD.d" "TFT_LCD_Lib\ColorTFTSymbols.d" "TFT_LCD_Lib\IFTSPI2_2LCD.d" "third_party\fatfs\port\mmc-dk-tm4c123g.d" "third_party\fatfs\src\ff.d" "third_party\fatfs\src\option\cc932.d" "utils\cmdline.d" 
	-$(RM) "AD9833.obj" "ADS1241.obj" "DS3231.obj" "DigitalFilter_Lib.obj" "Viettel_Tiva123_BlackBox.obj" "tm4c123gh6pm_startup_ccs.obj" "CAM\CAM.obj" "Cellular\cellular.obj" "MQTT\MQTT_HwLayer.obj" "NetworkModule\AT_Function.obj" "NetworkModule\GNSS.obj" "NetworkModule\GPS.obj" "NetworkModule\uartstdio.obj" "RFID\RFID.obj" "SD\SD.obj" "TFT_LCD_Lib\ColorTFTSymbols.obj" "TFT_LCD_Lib\IFTSPI2_2LCD.obj" "third_party\fatfs\port\mmc-dk-tm4c123g.obj" "third_party\fatfs\src\ff.obj" "third_party\fatfs\src\option\cc932.obj" "utils\cmdline.obj" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

