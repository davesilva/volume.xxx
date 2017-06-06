$(document).ready(function() {
    var $power = $("input[name=power]");
    var $volume = $("input[name=volume]");

    var getSpeakerState = function() {
        $.get("/speakers").done(function(response) {
            $power.val([response["power"]]);
            $volume.val(response["volume"]);
        });
    };

    var setSpeakerState = function() {
        $.post("/speakers", {
            power: $power.filter(":checked").val(),
            volume: $volume.val()
        });
    };

    var sendIrCode = function(code) {
        $.post("/ir", {
            code: code
        }).then(function(response) {
            console.log("Send IR code: " + code);
        });
    };

    var projectorPower = function() {
        sendIrCode("0x10C8E11E");
    };

    $volume.TouchSpin({
        min: 0,
        max: 70,
        step: 1,
        booster: false,
        mousewheel: false
    });

    $volume.change(function(event) {
        $power.val([true]);
    });

    $volume.on("mouseup", function(event) {
        $volume.select();
    });

    $("#speakers_form input").change(function(event) {
        setSpeakerState();
        return false;
    });

    $("#projector_on").click(function(event) {
        projectorPower();
        event.preventDefault();
    });

    $("#projector_off").click(function(event) {
        projectorPower();
        window.setTimeout(function() {
            projectorPower();
        }, 500);
        event.preventDefault();
    });

    $("#hdmi_input_1").change(function(event) {
        sendIrCode("0xFF00FF");
    });

    $("#hdmi_input_2").change(function(event) {
        sendIrCode("0xFF807F");
    });

    $("#hdmi_input_3").change(function(event) {
        sendIrCode("0xFF40BF");
    });

    $("#hdmi_input_4").change(function(event) {
        sendIrCode("0xFFC03F");
    });

    getSpeakerState();

    $volume.focus();
    $("input[type=submit]").hide();
});
