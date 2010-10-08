
var ideal_interval = 200;

var maximum_interval = 5000;

var interval = ideal_interval;

function grab_time() {

   var d = new Date();
   return d.getTime();

}

var timeStart = grab_time(), error_timeEnd, success_timeEnd, timeAvg = 0;

function reoptimize_interval () {

    if (timeAvg > ideal_interval) {

	if (ideal_interval + 5 < timeAvg + 5) {
  
	    ideal_interval += 5;

	}

    }

    else if (timeAvg < ideal_interval) {

	if (ideal_interval - 5 > timeAvg + 5) {

	    ideal_interval -= 5; 

	}

    }

 
    if (ideal_interval > interval) {

	if (interval + 5 < ideal_interval) {

	    interval+=5;

	}

    }

    else if (ideal_interval < interval) {

	if (interval - 5 > ideal_interval) {
		       
	    interval -= 5;

	}

    }

}



