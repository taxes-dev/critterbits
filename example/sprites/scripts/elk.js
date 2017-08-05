// declare module
var elk = (function() {
var em = {};

var VELOCITY = 60;
var MOVE_LENGTH = 2;
var flipped = false;

em.start = function() {
    var callback = function() {
        var dest = { "x": this.pos.x, "y": this.pos.y };
        flipped = !flipped;
        if (flipped) {
            this.animation.play("walk_left");
            dest.x -= VELOCITY * MOVE_LENGTH;
        } else {
            this.animation.play("walk_right");
            dest.x += VELOCITY * MOVE_LENGTH;
        }
        this.move_to(dest, MOVE_LENGTH * 1000, "lerp", callback);
    };
    callback.call(this);
}

//FIXME: callbacks don't get called if there's no update function
em.update = function(dt) {
}

// end module
return em;
}());