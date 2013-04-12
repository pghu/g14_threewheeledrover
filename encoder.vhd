-----------------------------------------------------------------
--File: encoder.vhd
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

entity encoder is

	port(
		csi_myclock_clk				:	in		std_logic;
		csi_myclock_reset			:	in		std_logic;

		avs_encoder_read_n			:	in		std_logic;
		avs_encoder_chipselect		:	in		std_logic;
		avs_encoder_address			:	in		std_logic_vector(1 downto 0);
		avs_encoder_readdata		:	out	    std_logic_vector(31 downto 0);
		avs_encoder_writedata		:	in		std_logic_vector(31 downto 0);

		coe_encoder_input_export	:	in		std_logic
	);

end entity;

architecture encoder_control of encoder is
	signal counter : std_logic_vector(31 downto 0) := (others => '0'); --32 bit counter
begin
	main_proc:process(csi_myclock_clk,csi_myclock_reset)
	begin
		if csi_myclock_clk'event and csi_myclock_clk = '1' then
			if std_logic'(((avs_encoder_chipselect AND NOT avs_encoder_read_n) AND to_std_logic((((std_logic_vector'("000000000000000000000000000000") & (avs_encoder_address)) = std_logic_vector'("00000000000000000000000000000000")))))) = '1' then
				avs_encoder_readdata <= counter;
			end if;
		end if;
	end process main_proc;

	clk_proc:process(coe_encoder_input_export)
	begin
		if rising_edge(coe_encoder_input_export) then
			counter <= counter + 1; --increment counter on rising edge
		end if;
	end process clk_proc;
end encoder_control;