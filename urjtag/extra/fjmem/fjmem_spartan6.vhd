-------------------------------------------------------------------------------
--
-- $Id$
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; either version 2
-- of the License, or (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
-- 02111-1307, USA.
--
-- Written by Michael Walle <michael@walle.cc>, 2010.
-- Based on fjmem_spartan3.vhd by Arnim Laeuger.
--
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

entity fjmem_spartan6 is

  port (
    -- Clock oscillator
    osc1                     : in    std_logic;
    
    -- Flash Memory
    fl_a                     : out   std_logic_vector(23 downto 0);
    fl_d                     : inout std_logic_vector(15 downto 0);
    fl_ce_n                  : out   std_logic;
    fl_oe_n                  : out   std_logic;
    fl_we_n                  : out   std_logic;
    fl_rp_n                  : out   std_logic;
    fl_sts                   : in    std_logic;

    led                      : out   std_logic_vector(1 downto 0)
  );

end fjmem_spartan6;


library ieee;
use ieee.numeric_std.all;

use work.fjmem_config_pack.all;

architecture struct of fjmem_spartan6 is

  component fjmem_core
    port (
      clkdr_i  : in  std_logic;
      trst_i   : in  std_logic;
      shift_i  : in  std_logic;
      update_i : in  std_logic;
      tdi_i    : in  std_logic;
      tdo_o    : out std_logic;
      clk_i    : in  std_logic;
      res_i    : in  std_logic;
      strobe_o : out std_logic;
      read_o   : out std_logic;
      write_o  : out std_logic;
      ack_i    : in  std_logic;
      cs_o     : out std_logic_vector(num_blocks_c-1 downto 0);
      addr_o   : out std_logic_vector(max_addr_width_c-1 downto 0);
      din_i    : in  std_logic_vector(max_data_width_c-1 downto 0);
      dout_o   : out std_logic_vector(max_data_width_c-1 downto 0)
    );
  end component;

  component BSCAN_SPARTAN6
    port (
      CAPTURE : out std_ulogic := 'H';
      DRCK    : out std_ulogic := 'L';
      RESET   : out std_ulogic := 'L';
      RUNTEST : out std_ulogic := 'L';
      SEL     : out std_ulogic := 'L';
      SHIFT   : out std_ulogic := 'L';
      TCK     : out std_ulogic := 'L';
      TDI     : out std_ulogic := 'L';
      TMS     : out std_ulogic := 'L';
      UPDATE  : out std_ulogic := 'L';
      TDO     : in  std_ulogic := 'X'
    );
  end component;


  signal tdi_s,
         tdo_s    : std_logic;
  signal clkdr_s,
         trst_s,
         shift_s,
         update_s : std_logic;

  signal addr_s   : std_logic_vector(max_addr_width_c-1 downto 0);
  signal din_s,
         dout_s   : std_logic_vector(max_data_width_c-1 downto 0);

  signal res_s    : std_logic;

  signal read_s,
         write_s,
         strobe_s : std_logic;
  signal cs_s     : std_logic_vector(1 downto 0);
  signal ack_q    : std_logic;

  type   state_t is (IDLE,
                     READ_WAIT,
                     WRITE_DRIVE,
                     WRITE_WAIT,
                     WRITE_FINISH);
  signal state_q : state_t;

  signal cnt_q : unsigned(2 downto 0);

  signal fl_ce_n_q,
         fl_oe_n_q,
         fl_we_n_q,
         fl_d_en_q    : std_logic;

begin

  res_s <= '0';


  bscan_spartan6_b : BSCAN_SPARTAN6
    port map (
      CAPTURE => open,
      DRCK    => clkdr_s,
      RESET   => open, --trst_s,
      RUNTEST => open,
      SEL     => open,
      SHIFT   => shift_s,
      TCK     => open,
      TDI     => tdi_s,
      TMS     => open,
      UPDATE  => update_s,
      TDO     => tdo_s
    );
  trst_s <= '0';


  fjmem_core_b : fjmem_core
    port map (
      clkdr_i  => clkdr_s,
      trst_i   => trst_s,
      shift_i  => shift_s,
      update_i => update_s,
      tdi_i    => tdi_s,
      tdo_o    => tdo_s,
      clk_i    => osc1,
      res_i    => res_s,
      strobe_o => strobe_s,
      read_o   => read_s,
      write_o  => write_s,
      ack_i    => ack_q,
      cs_o     => cs_s,
      addr_o   => addr_s,
      din_i    => din_s,
      dout_o   => dout_s
    );


  -----------------------------------------------------------------------------
  -- Process mem_ctrl
  --
  -- Purpose:
  --   Handles access to external memory.
  --
  mem_ctrl: process (res_s, osc1)
  begin
    if res_s = '1' then
      -- Flash
      fl_ce_n_q   <= '1';
      fl_oe_n_q   <= '1';
      fl_we_n_q   <= '1';
      fl_d_en_q   <= '0';

      ack_q <= '0';

      state_q     <= IDLE;
      cnt_q       <= (others => '0');

    elsif rising_edge(osc1) then
      case state_q is
        when IDLE =>
          if strobe_s = '1' then
            if write_s = '1' then
              state_q <= WRITE_DRIVE;
            else
              state_q <= READ_WAIT;
              ack_q   <= '1';
            end if;

            case cs_s is
              -- Flash
              when "01" =>
                fl_ce_n_q   <= '0';
                if read_s = '1' then
                  fl_oe_n_q <= '0';
                  -- start counter on read
                  cnt_q     <= (others => '1');
                end if;
                if write_s = '1' then
                  fl_d_en_q <= '1';
                end if;

              -- unimlemented / invalid
              when others =>
                null;

            end case;
          end if;

        when READ_WAIT =>
          if cnt_q = 0 then
            state_q <= IDLE;
            ack_q   <= '0';

            -- disable all memories
            fl_ce_n_q  <= '1';
            fl_oe_n_q  <= '1';
          end if;

        when WRITE_DRIVE =>
          state_q <= WRITE_WAIT;

          -- output drivers are active during this state
          -- thus we can activate the write impulse at the end
          case cs_s is
            when "01" =>
              fl_we_n_q <= '0';
              -- start counter
              cnt_q     <= (others => '1');
            when others =>
              null;
          end case;

        when WRITE_WAIT =>
          if cnt_q = 0 then
            state_q <= WRITE_FINISH;
            ack_q   <= '0';

            -- disable write signals
            fl_we_n_q  <= '1';
          end if;

        when WRITE_FINISH =>
          state_q <= IDLE;

          -- disable output enables
          fl_d_en_q  <= '0';
          -- disable all memories
          fl_ce_n_q  <= '1';
          fl_oe_n_q  <= '1';

        when others =>
          state_q <= IDLE;

      end case;

      if cnt_q /= 0 then
        cnt_q <= cnt_q - 1;
      end if;

    end if;
  end process mem_ctrl;
  --
  -----------------------------------------------------------------------------


  -----------------------------------------------------------------------------
  -- Process read_mux
  --
  -- Purpose:
  --   Read multiplexer from memory to jop_core.
  --
  read_mux: process (cs_s, fl_d)
    variable din_v : std_logic_vector(din_s'range);
  begin
    din_v := (others => '0');

    if cs_s(0) = '1' then
      din_v := din_v or fl_d;
    end if;

    din_s <= din_v;
  end process read_mux;
  --
  -----------------------------------------------------------------------------


  -----------------------------------------------------------------------------
  -- Pin defaults
  -----------------------------------------------------------------------------
  -- Flash Memory -------------------------------------------------------------
  fl_addr: process (addr_s)
  begin
  --  fl_a <= (others => '0');
    fl_a(23 downto 0) <= addr_s;
  end process fl_addr;
  fl_d      <=   dout_s
               when fl_d_en_q = '1' else
                 (others => 'Z');
  fl_ce_n   <= fl_ce_n_q;
  fl_oe_n   <= fl_oe_n_q;
  fl_we_n   <= fl_we_n_q;
  fl_rp_n   <= '1';

  -- LEDs
  led(0) <= fl_we_n_q;
  led(1) <= fl_oe_n_q;

end struct;
