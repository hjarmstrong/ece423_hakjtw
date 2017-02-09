// rgb_acel.v

// This file was auto-generated as a prototype implementation of a module
// created in component editor.  It ties off all outputs to ground and
// ignores all inputs.  It needs to be edited to make it do something
// useful.
// 
// This file will not be automatically regenerated.  You should check it in
// to your version control system if you want to keep it.

`timescale 1 ps / 1 ps
module rgb_acel (
		input  wire [31:0] cr_in_data,    //   cr_in.data
		output wire        cr_in_ready,   //        .ready
		input  wire        cr_in_valid,   //        .valid
		input  wire [31:0] cb_in_data,    //   cb_in.data
		output wire        cb_in_ready,   //        .ready
		input  wire        cb_in_valid,   //        .valid
		output wire        y_in_ready,    //    y_in.ready
		input  wire        y_in_valid,    //        .valid
		input  wire [31:0] y_in_data,     //        .data
		input  wire        clock_clk,     //   clock.clk
		output wire [31:0] rgb_out_data,  // rgb_out.data
		input  wire        rgb_out_ready, //        .ready
		output wire        rgb_out_valid, //        .valid
		input  wire        reset          //   reset.reset
	);

	// TODO: Auto-generated HDL template

	assign cr_in_ready = 1'b0;

	assign cb_in_ready = 1'b0;

	assign y_in_ready = 1'b0;

	assign rgb_out_valid = 1'b0;

	assign rgb_out_data = 32'b00000000000000000000000000000000;

endmodule
