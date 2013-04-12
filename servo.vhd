-----------------------------------------------------------------
--File: servo.vhd
--Author: Peter Hu
--Group: g14 Three-Wheeled Rover 2013
--Source: Modified from PWM.vhd code which is based on 
--        g1 iOS Remote Control Car appnote code by Robert Hood. 
--Original date: Feb 27, 2011
-----------------------------------------------------------------
library altera;
use altera.altera_europa_support_lib.all;

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity servo is

	port(
		csi_myclock_clk			:	in		std_logic;
		csi_myclock_reset		:	in		std_logic;

		avs_pwm_write_n			:	in		std_logic;
		avs_pwm_chipselect		:	in		std_logic;
		avs_pwm_address			:	in		std_logic_vector(1 downto 0);
		avs_pwm_readdata		:	out	    std_logic_vector(31 downto 0);
		avs_pwm_writedata		:	in		std_logic_vector(31 downto 0);

		coe_pwm_output_export	:	out		std_logic
	);

end entity;

architecture servo_control of servo is

	signal servo_signal				:	std_logic_vector(19 downto 0); --Represents the duty cycle
	signal counter					:	std_logic_vector(19 downto 0) := (others => '0'); --20 bit counter

begin

	main_proc:process(csi_myclock_clk,csi_myclock_reset)
	begin
		if csi_myclock_reset= '1' then
			servo_signal <= (others => '0');
		elsif csi_myclock_clk'event and csi_myclock_clk = '1' then
			if std_logic'(((avs_pwm_chipselect AND NOT avs_pwm_write_n) AND to_std_logic((((std_logic_vector'("000000000000000000000000000000") & (avs_pwm_address)) = std_logic_vector'("00000000000000000000000000000000")))))) = '1' then
				servo_signal <= avs_pwm_writedata(19 DOWNTO 0);
			end if;
		end if;
	end process main_proc;

	clk_proc:process(csi_myclock_clk)
	begin
		if csi_myclock_reset='1' then
			counter <= (others => '0');
		elsif rising_edge(csi_myclock_clk) then
			counter <= counter + 1;
		end if;

--target output is 50 Hz, want to use the 50MHz clock
--50 MHz / 50 Hz = 1 000 000 clock ticks per period  =  1111 0100 0010 0100 0000
		if (counter = "11110100001001000000") then
			counter <= (others => '0');
		end if;
	
		if ((servo_signal>counter)and (servo_signal>0)) then
			coe_pwm_output_export <= '1';
		else
			coe_pwm_output_export <= '0';
		end if;
	end process clk_proc;
end servo_control;