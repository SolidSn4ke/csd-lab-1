################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/MyDriver/Src/mydriver.c 

OBJS += \
./Drivers/MyDriver/Src/mydriver.o 

C_DEPS += \
./Drivers/MyDriver/Src/mydriver.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/MyDriver/Src/%.o Drivers/MyDriver/Src/%.su: ../Drivers/MyDriver/Src/%.c Drivers/MyDriver/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F427xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-MyDriver-2f-Src

clean-Drivers-2f-MyDriver-2f-Src:
	-$(RM) ./Drivers/MyDriver/Src/mydriver.d ./Drivers/MyDriver/Src/mydriver.o ./Drivers/MyDriver/Src/mydriver.su

.PHONY: clean-Drivers-2f-MyDriver-2f-Src

