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

component idct_1D
port(
	signal clk: in std_logic;				-- CPU system clock (always required)
	
	signal pass : in std_logic;			-- 0: Pass 1; 1: Pass 2
	
	signal i0: in std_logic_vector(15 downto 0);
	signal i1: in std_logic_vector(15 downto 0);
	signal i2: in std_logic_vector(15 downto 0);
	signal i3: in std_logic_vector(15 downto 0);
	signal i4: in std_logic_vector(15 downto 0);
	signal i5: in std_logic_vector(15 downto 0);
	signal i6: in std_logic_vector(15 downto 0);
	signal i7: in std_logic_vector(15 downto 0);
	
	signal o0: out std_logic_vector(15 downto 0);
	signal o1: out std_logic_vector(15 downto 0);
	signal o2: out std_logic_vector(15 downto 0);
	signal o3: out std_logic_vector(15 downto 0);
	signal o4: out std_logic_vector(15 downto 0);
	signal o5: out std_logic_vector(15 downto 0);
	signal o6: out std_logic_vector(15 downto 0);
	signal o7: out std_logic_vector(15 downto 0)
);
end component;

    signal dataBuffer: STD_LOGIC_VECTOR(31 downto 0);
	 signal state: std_logic_vector(1 downto 0);
	
	type worker_mat_t is array (0 to 7, 0 to 7) of std_logic_vector(15 downto 0);
   signal workspace: worker_mat_t;
	signal input_buffer_holder: worker_mat_t;
	signal output_buffer_holder: worker_mat_t;
	
	signal hwin: worker_mat_t;
	signal hwout: worker_mat_t;
	
	signal idct_row_count: integer range 0 to 15 := 0;
	signal idct_col_count: integer range 0 to 15 := 0;
	signal pass_wait_count: integer range 0 to 15 := 0;
	
	signal pass_sel : std_logic := '0';			-- 0: Pass 1; 1: Pass 2
begin

   dst_data <= dataBuffer;
	
	-- generate accelerator port maps
	acelplace: for I in 0 to 7 generate
	    hwacel: idct_1D PORT MAP(
			clk => clk,
			pass => pass_sel,
			
			i0 => hwin(I,0),
			i1 => hwin(I,1),
			i2 => hwin(I,2),
			i3 => hwin(I,3),
			i4 => hwin(I,4),
			i5 => hwin(I,5),
			i6 => hwin(I,6),
			i7 => hwin(I,7),
			
			o0 => hwout(I,0),
			o1 => hwout(I,1),
			o2 => hwout(I,2),
			o3 => hwout(I,3),
			o4 => hwout(I,4),
			o5 => hwout(I,5),
			o6 => hwout(I,6),
			o7 => hwout(I,7)
	
		);
	end generate acelplace;
	
	-- generate pass io on the hwacels
	passselcol: for I in 0 to 7 generate
	    passselrow: for J in 0 to 7 generate
		    
			hwin(I,J) <= input_buffer_holder(I,J) when pass_sel = '0' else
	        workspace(J,I);
			
			process(pass_sel, hwout(I,J)) begin
				if (pass_sel = '0') then
					workspace(I,J) <= hwout(I,J); 
				else
					output_buffer_holder(J,I) <= hwout(I,J);
				end if; 
			end process;
			
		end generate passselrow;
	end generate passselcol;
	 
	 -- And finally use the fsm
	 process(clk, reset) begin
	 if (reset = '1') then
	     src_ready <= '1';
		  dst_valid <= '0';
		  state <= "00"; -- Read Data
		  idct_row_count <= 0;
		  idct_col_count <= 0;
		  pass_wait_count <= 0;
                  pass_sel <= '0';
	 elsif (rising_edge(clk)) then
	     if (state = "00") then
		      if (src_valid = '1') then
				    --input_buffer_holder(idct_col_count, idct_row_count) <= src_data(15 downto 0);
					--input_buffer_holder(idct_col_count, idct_row_count + 1) <= src_data(31 downto 16);
					-- Little endian fix? 
					input_buffer_holder(idct_col_count, idct_row_count) <= (src_data(15 downto 8) & src_data(7 downto 0));
					input_buffer_holder(idct_col_count, idct_row_count + 1) <= src_data(31 downto 23) & src_data(22 downto 16) ;
					if ((idct_col_count = 7) and (idct_row_count = 6)) then
					    state <= "01"; -- wait/do operation...
					    src_ready <= '0';
					    idct_row_count <= 0;
					    idct_col_count <= 0;					
					elsif (idct_row_count = 6) then
						idct_row_count <= 0;
						idct_col_count <= idct_col_count + 1;
				    else
					    idct_row_count <= idct_row_count + 2;
					end if;
			   end if;
		  elsif (state = "01") then
		      if (pass_wait_count = 5) then
			      state <= "10"; -- idct pass 2
				  pass_wait_count <= 0;
				  pass_sel <= '1';
			   else
			     pass_wait_count <= pass_wait_count + 1;
			  end if;
		  elsif (state = "10") then
		  	if (pass_wait_count = 5) then
			          state <= "11"; -- transmit
				  pass_wait_count <= 0;
                                  dst_valid <= '1';
				  dataBuffer <= output_buffer_holder(idct_col_count, idct_row_count)(7 downto 0) & 
				  output_buffer_holder(idct_col_count, idct_row_count + 1)(7 downto 0) & 
				  output_buffer_holder(idct_col_count, idct_row_count + 2)(7 downto 0) &
				  output_buffer_holder(idct_col_count, idct_row_count + 3)(7 downto 0);
                                  idct_row_count <= 4;
			   else
			     pass_wait_count <= pass_wait_count + 1;
			  end if;
		  elsif (state = "11") then
		      if (dst_ready = '1') then
			      -- Endianness should not matter here... hopefully
				  dst_valid <= '1';
				  dataBuffer <= output_buffer_holder(idct_col_count, idct_row_count)(7 downto 0) & 
				  output_buffer_holder(idct_col_count, idct_row_count + 1)(7 downto 0) & 
				  output_buffer_holder(idct_col_count, idct_row_count + 2)(7 downto 0) &
				  output_buffer_holder(idct_col_count, idct_row_count + 3)(7 downto 0);
				  
				  if ((idct_col_count = 7) and (idct_row_count = 4)) then
					  dst_valid <= '0';
					  src_ready <= '1';
					  idct_row_count <= 0;
					  idct_col_count <= 0;
					  pass_sel <= '0';
					  state <= "00";
					  elsif (idct_row_count = 4) then
						idct_row_count <= 0;
						idct_col_count <= idct_col_count + 1;
				    else
					    idct_row_count <= idct_row_count + 4;
				  end if;
			   end if;
		  end if;
	 end if;
	 end process;
	 
end architecture rtl; -- of idct_accel

