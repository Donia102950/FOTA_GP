################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ECUAL/USART_Handler/hUART_Config.c \
../src/ECUAL/USART_Handler/hUSART.c 

OBJS += \
./src/ECUAL/USART_Handler/hUART_Config.o \
./src/ECUAL/USART_Handler/hUSART.o 

C_DEPS += \
./src/ECUAL/USART_Handler/hUART_Config.d \
./src/ECUAL/USART_Handler/hUSART.d 


# Each subdirectory must supply rules for building sources it contributes
src/ECUAL/USART_Handler/%.o: ../src/ECUAL/USART_Handler/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"G:\FOTA_GP\BootLoader\BL_project\src\Application" -I"G:\FOTA_GP\BootLoader\BL_project\src\ECUAL\RCC_Handler" -I"G:\FOTA_GP\BootLoader\BL_project\src\ECUAL\USART_Handler" -I"G:\FOTA_GP\BootLoader\BL_project\src\Libraries" -I"G:\FOTA_GP\BootLoader\BL_project\src\Libraries\ARM_Delay" -I"G:\FOTA_GP\BootLoader\BL_project\src\MCAL\Flash" -I"G:\FOTA_GP\BootLoader\BL_project\src\MCAL\GPIO" -I"G:\FOTA_GP\BootLoader\BL_project\src\MCAL\NVIC" -I"G:\FOTA_GP\BootLoader\BL_project\src\MCAL\RCC" -I"G:\FOTA_GP\BootLoader\BL_project\src\MCAL\USART" -I"G:\FOTA_GP\BootLoader\BL_project\src\Service\TransportProtocol" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


