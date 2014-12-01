$(document).ready(function() {
    var $power = $("input[name=power]");
    var $volume = $("input[name=volume]");
    var $form = $('form');

    var getSpeakerState = function() {
        $.ajax("/speakers").done(function(response) {
            $power.val([response['power']]);
            $volume.val(response['volume']);
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

    $('input').change(function(event) {
        $form.submit();
    });

    getSpeakerState();

    $volume.focus();
    $('input[type=submit]').hide();
});
