var max_val = 160;
var min_val = 0;
var max_deg = 340;
var min_deg = 20;
var max_hz;
var min_hz;
var cur_hz;
var heater_on;
var run_state;
var body_temp;


$(document).ready(function(){	
	max_hz = parseFloat($("#max_hz").val());
	min_hz = parseFloat($("#min_hz").val());
	cur_hz = parseFloat($("#cur_hz").val());
	heater_on = parseInt($("#on_off").val());
	run_state = $("#run_state").val();
	body_temp = parseInt($("#body_temp").val());
	
	if(heater_on) {
		$(this).removeClass("off");
		$(this).addClass("on");
	}


	$(".pump_hz .num").text(cur_hz.toFixed(1));
	$("#run_state_disp").text(run_state);
	
	update_body_temp_guage();
	
	var timeout_interval = setInterval(function() {refresh_controller();},5000);
	
	var on_off_timeout;
	$(".controller").click(function(e) {  
		if(!heater_on && $(this).hasClass("off")) {
			$(this).addClass("turningon");
			$(this).removeClass("off");
			on_off_timeout = setTimeout(function() {delay_on();}, 3000); // allow 3 seconds to be turned off again, in case it's accidental - the heater isn't so forgiving!
		}
		else if($(this).hasClass("turningon")){
			$(this).removeClass("turningon");
			clearTimeout(on_off_timeout);
		}
		
		if(heater_on && $(this).hasClass("on")){
			$(this).addClass("turningoff");
			$(this).removeClass("on");
			on_off_timeout = setTimeout(function() {delay_off();}, 3000); // allow 3 seconds to be turned off again, in case it's accidental - the heater isn't so forgiving!
		}
		else if($(this).hasClass("turningoff")){
			$(this).removeClass("turningoff");
			clearTimeout(on_off_timeout);
		}
	});	
	
	$(".controller .plus").click(function(e) {  
		e.stopPropagation();
		cur_hz += 0.1;
		if(cur_hz > max_hz) {
			cur_hz = max_hz;
			return;
		}
		$(".pump_hz .num").text(cur_hz.toFixed(1));
		$.post( "/", { hz: cur_hz } );
	});
	
	$(".minus").click(function(e) {  
		e.stopPropagation();
		cur_hz -= 0.1;
		if(cur_hz < min_hz) {
			cur_hz = min_hz;
			return;
		}
		$(".pump_hz .num").text(cur_hz.toFixed(1));
		$.post( "/", { hz: cur_hz } );
	});
});


function scale(num, in_min, in_max, out_min, out_max) {
  return (num - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

function refresh_controller(){
	$('#heater_vars').load(location.href + " #heater_vars", function( response, status, xhr ) {
		run_state = $("#run_state").val();
		$("#run_state_disp").text(run_state);
		
		heater_on = parseInt($("#on_off").val());
		if(heater_on) {
			$(".controller").removeClass("turningon");
			$(".controller").removeClass("turningoff");
			$(".controller").removeClass("off");
			$(".controller").addClass("on");
		}
		else {
			$(".controller").removeClass("turningon");
			$(".controller").removeClass("turningoff");
			$(".controller").removeClass("on");
			$(".controller").addClass("off");
		}
	});
	
	body_temp = parseInt($("#body_temp").val());
	update_body_temp_guage();
}

function delay_on() {
	if($('.controller').hasClass("turningon")) {
		$.post( "/", { state: "on" } );
	}
}

function delay_off() {
	if($('.controller').hasClass("turningoff")) {
		$.post( "/", { state: "off" } );
	}
}

function update_body_temp_guage() {
	if(body_temp < min_val)
		body_temp = min_val;
	if(body_temp > max_val)
		body_temp = max_val;
	
	$(".body_temp").css("transform", "translate(-50%, -50%) rotate("+ (-180 + scale(body_temp, min_val, max_val, min_deg, max_deg))+"deg)");
	$(".fill").css("animation", "none");

	if(body_temp >= ((max_val - min_val) / 2)){
		$(".fill1").css("transform", "rotate("+ (scale(body_temp, min_val, max_val, min_deg, max_deg) - 180) +"deg)").css("transition-delay", "0s");
	}else{
		$(".fill2").css("transform", "rotate("+ scale(body_temp, min_val, max_val, min_deg, max_deg) +"deg)").css("transition-delay", "0s");
	}
	
	$(".body_temp .num").text(body_temp);
}