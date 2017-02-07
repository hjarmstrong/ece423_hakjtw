-- idct_accel.vhd

-- This file was auto-generated as a prototype implementation of a module
-- created in component editor.  It ties off all outputs to ground and
-- ignores all inputs.  It needs to be edited to make it do something
-- useful.
-- 
-- This file will not be automatically regenerated.  You should check it in
-- to your version control system if you want to keep it.

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity idct_accel is
	port (
		clk       : in  std_logic                     := '0';             -- clock_sink.clk
		src_data  : in  std_logic_vector(31 downto 0) := (others => '0'); --        src.data
		src_ready : out std_logic;                                        --           .ready
		src_valid : in  std_logic                     := '0';             --           .valid
		reset     : in  std_logic                     := '0';             -- reset_sink.reset
		dst_data  : out std_logic_vector(31 downto 0);                    --        dst.data
		dst_ready : in  std_logic                     := '0';             --           .ready
		dst_valid : out std_logic                                         --           .valid
	);
end entity idct_accel;

architecture rtl of idct_accel is
    signal dataBuffer: STD_LOGIC_VECTOR(31 downto 0);
	 signal state: std_logic_vector(1 downto 0);
begin

    dst_data <= dataBuffer;
	 
	 process(clk, reset) begin
	 if (reset = '1') then
	     src_ready <= '1';
		  dst_valid <= '0';
		  state <= "00"; -- Read Data
	 elsif (rising_edge(clk)) then
	     if (state = "00") then
		      if (src_valid = '1') then
				    dataBuffer <= src_data;
					 src_ready <= '0';
					 state <= "01"; -- wait/do operation...
			   end if;
		  elsif (state = "01") then
		      dataBuffer <= (NOT dataBuffer);
				state <= "10"; -- transmit
				dst_valid <= '1';
		  elsif (state = "10") then
		      if (dst_ready = '1') then
		          dst_valid <= '0';
					 src_ready <= '1';
					 state <= "00";
			   end if;
		  end if;
	 end if;
	 end process;
	 
end architecture rtl; -- of idct_accel
