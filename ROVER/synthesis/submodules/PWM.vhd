-----------------------------------------------------
--File: PWM.vhd
--Author: Peter Hu
--Group: g14 Three-Wheeled Rover 2013
--Source: Modified from g1 iOS Remote Control Car appnote code by Robert Hood. Original date: Feb 27, 2011
-----------------------------------------------------
library altera;
use altera.altera_europa_support_lib.all;

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity pwm is

port(
     
     csi_myclock_clk 	   : in std_logic; --clk is standard for both PWM and Avalon memory slave
     csi_myclock_reset	   : in std_logic; --clk reset
	 
     avs_pwm_write_n 	   : in std_logic; --write
     avs_pwm_chipselect	   : in std_logic; -- chip select
     avs_pwm_address 	   : in std_logic_vector(1 downto 0); --address
     avs_pwm_readdata 	   : out std_logic_vector(15 downto 0); --Needed for the Avalon interface
     avs_pwm_writedata 	   : in std_logic_vector(15 downto 0); -- Data to be input represents the duty cycle

     coe_pwm_output_export : out std_logic-- Ouput either 1 (high) or 0 (low)	
);

end entity;

architecture pwm_control of pwm is

signal pwm_signal : std_logic_vector(19 downto 0); --represents the duty cycle
signal counter : std_logic_vector(19 downto 0) := (others => '0'); -- 20 bit counter

begin
	main_proc:process(csi_myclock_clk,csi_myclock_reset)
	begin
		--if reset than set the duty cycle to '0'
		if csi_myclock_reset= '1' then
			pwm_signal <= (others => '0');
		
   --If reset is not set to '1' and chipselect, write_n and address are the correct values
   -- Then set the duty cycle to the value that was input by the user
		elsif csi_myclock_clk'event and csi_myclock_clk = '1' then
			if std_logic'(((avs_pwm_chipselect AND NOT avs_pwm_write_n) AND to_std_logic((((std_logic_vector'("000000000000000000000000000000") & (avs_pwm_address)) = std_logic_vector'("00000000000000000000000000000000")))))) = '1' then
				pwm_signal <= "0000" & avs_pwm_writedata(15 DOWNTO 0);
			end if;
		end if;
	end process main_proc;

	clk_proc:process(csi_myclock_clk)
--------Increment the counter
	begin
		if csi_myclock_reset='1' then
			counter <= (others => '0');
--At every rising clock edge, increment the counter 
		elsif rising_edge(csi_myclock_clk) then
			counter <= counter +1;
		end if;

--target output is 1600 Hz, want to use the 50MHz clock
--50 MHz / 1600 Hz = 31250 = 0000111101000010010
		if(counter = "0000111101000010010") then
			counter <= (others => '0');
		end if;
			
--For as long as the "pwm_signal" value is greater than 0, and greater than the "counter" value, 
--set the "pwm_output" to 1(high)
--Else set "pwm_output" it to 0(low)
		if ((pwm_signal>counter) and (pwm_signal>0)) then		   
			coe_pwm_output_export<='1';
		else 
			coe_pwm_output_export<='0'; 
		end if;
	end process clk_proc;

end pwm_control;