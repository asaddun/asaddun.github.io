function openNav() {
    document.getElementById("mySidebar").style.width = "300px";
}
  
function closeNav() {
    document.getElementById("mySidebar").style.width = "0";
}

function toggleButton() {
    var toggleBtn = document.getElementById("toggleBtn");
    var toggleValue = document.getElementById("toggleValue");
    if (toggleBtn.checked) {
      toggleValue.value = "1";
    } else {
      toggleValue.value = "0";
    }
}

var button = document.getElementById("toggleBtn");
var value = document.getElementById("toggleValue");
if (button.checked) {
} else {
    value.value = "0";
}

function selectInterval() {
    var interval = document.getElementById("intervalSelect");
    var numberInterval = document.getElementById("numberInterval");
    var selectedInterval = interval.value;
    if (selectedInterval == "3" && numberInterval.value >= 24) {
      numberInterval.value = 23;
    } else {}

    var nameInterval = document.getElementById("intervalSelected");
    nameInterval.value = selectedInterval;
}

function selectTime1() {
    var hour1 = document.getElementById("hourSelect1");
    var minute1 = document.getElementById("minuteSelect1");
    var selectedHour1 = hour1.options[hour1.selectedIndex].text;
    var selectedMinute1 = minute1.options[minute1.selectedIndex].text;

    var nameHour1 = document.getElementById("hourSelected1");
    var nameMinute1 = document.getElementById("minuteSelected1");
    nameHour1.value = selectedHour1;
    nameMinute1.value = selectedMinute1;;
}

function selectTime2() {
    var hour2 = document.getElementById("hourSelect2");
    var minute2 = document.getElementById("minuteSelect2");
    var selectedHour2 = hour2.options[hour2.selectedIndex].text;
    var selectedMinute2 = minute2.options[minute2.selectedIndex].text;

    var nameHour2 = document.getElementById("hourSelected2");
    var nameMinute2 = document.getElementById("minuteSelected2");
    nameHour2.value = selectedHour2;
    nameMinute2.value = selectedMinute2;
}

function goBack() {
    window.location.href = "/";
}

function confirmReset() {
    if (confirm("Are you sure you want to reset?")) {
        window.location.href = "/doreset";
    } else {
    }
}
