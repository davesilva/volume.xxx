$(document).ready(function() {
    var $power = $("input[name=power]");
    var $volume = $("input[name=volume]");
    var $form = $("form");

    var getSpeakerState = function() {
        $.get("/speakers").done(function(response) {
            $power.val([response["power"]]);
            $volume.val(response["volume"]);
        });
    };

    var setSpeakerState = function() {
        $.post("/speakers", {
            power: $power.find(":checked").val(),
            volume: $volume.val()
        });
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

    $("input").change(function(event) {
        $form.submit();
    });

    $form.submit(function(event) {
        setSpeakerState();
        event.preventDefault();
        return false;
    });

    getSpeakerState();

    $volume.focus();
    $("input[type=submit]").hide();
});
