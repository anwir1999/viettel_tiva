################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
TFT_LCD_Lib/ColorTFTSymbols.obj: ../TFT_LCD_Lib/ColorTFTSymbols.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/examples/boards/dk-tm4c123g" -g --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RA1 --define=PART_TM4C123GH6PM --diag_wrap=off --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="TFT_LCD_Lib/ColorTFTSymbols.d" --obj_directory="TFT_LCD_Lib" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

TFT_LCD_Lib/IFTSPI2_2LCD.obj: ../TFT_LCD_Lib/IFTSPI2_2LCD.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/examples/boards/dk-tm4c123g" -g --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RA1 --define=PART_TM4C123GH6PM --diag_wrap=off --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="TFT_LCD_Lib/IFTSPI2_2LCD.d" --obj_directory="TFT_LCD_Lib" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


