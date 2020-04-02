################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Common/%.obj: ../Common/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"/Applications/ti/ccs930/ccs/tools/compiler/ti-cgt-c2000_18.12.4.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --fp_mode=relaxed --include_path="/Users/Paiscaso/Desktop/Senior_Sandbox/sd_card" --include_path="/Applications/ti/c2000/C2000Ware_2_01_00_00/device_support/f2837xd/common/include" --include_path="/Applications/ti/c2000/C2000Ware_2_01_00_00/device_support/f2837xd/headers/include" --include_path="C:/ti/c2000/C2000Ware_2_00_00_02/libraries/dsp/FPU/c28/include/fpu32" --include_path="/Applications/ti/c2000/C2000Ware_2_01_00_00/libraries/dsp/FPU/c28/include/fpu32" --include_path="C:/ti/c2000/C2000Ware_2_00_00_02/libraries/dsp/FixedPoint/c28/include" --include_path="/Applications/ti/c2000/C2000Ware_2_01_00_00/libraries/dsp/FixedPoint/c28/include" --include_path="C:/ti/c2000/C2000Ware_2_00_00_02/device_support/f2837xd/headers/include" --include_path="C:/ti/c2000/C2000Ware_2_00_00_02/device_support/f2837xd/common/include" --include_path="/Applications/ti/ccs930/ccs/tools/compiler/ti-cgt-c2000_18.12.4.LTS/include" --define=CPU1 --define=_STANDALONE --define=_FLASH --define=_LAUNCHXL_F28379D -g --c99 --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="Common/$(basename $(<F)).d_raw" --obj_directory="Common" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Common/%.obj: ../Common/%.asm $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"/Applications/ti/ccs930/ccs/tools/compiler/ti-cgt-c2000_18.12.4.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --fp_mode=relaxed --include_path="/Users/Paiscaso/Desktop/Senior_Sandbox/sd_card" --include_path="/Applications/ti/c2000/C2000Ware_2_01_00_00/device_support/f2837xd/common/include" --include_path="/Applications/ti/c2000/C2000Ware_2_01_00_00/device_support/f2837xd/headers/include" --include_path="C:/ti/c2000/C2000Ware_2_00_00_02/libraries/dsp/FPU/c28/include/fpu32" --include_path="/Applications/ti/c2000/C2000Ware_2_01_00_00/libraries/dsp/FPU/c28/include/fpu32" --include_path="C:/ti/c2000/C2000Ware_2_00_00_02/libraries/dsp/FixedPoint/c28/include" --include_path="/Applications/ti/c2000/C2000Ware_2_01_00_00/libraries/dsp/FixedPoint/c28/include" --include_path="C:/ti/c2000/C2000Ware_2_00_00_02/device_support/f2837xd/headers/include" --include_path="C:/ti/c2000/C2000Ware_2_00_00_02/device_support/f2837xd/common/include" --include_path="/Applications/ti/ccs930/ccs/tools/compiler/ti-cgt-c2000_18.12.4.LTS/include" --define=CPU1 --define=_STANDALONE --define=_FLASH --define=_LAUNCHXL_F28379D -g --c99 --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="Common/$(basename $(<F)).d_raw" --obj_directory="Common" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


