// declare module
var player = (function() {
var pm = {};

pm.start = function() {
    viewport.follow(this);
}

var VELOCITY = 120;
pm.update = function(dt) {
    var vel_x = 0, vel_y = 0;
    if (input.is_direction_pressed(input.direction.LEFT)) {
        vel_x = VELOCITY * dt * -1;
    } else if (input.is_direction_pressed(input.direction.RIGHT)) {
        vel_x = VELOCITY * dt;
    }
    if (input.is_direction_pressed(input.direction.UP)) {
        vel_y = VELOCITY * dt * -1;
    } else if (input.is_direction_pressed(input.direction.DOWN)) {
        vel_y = VELOCITY * dt;
    }

    // prevent over-acceleration across diagonals
    if (vel_x != 0 && vel_y != 0) {
        vel_x *= 0.66;
        vel_y *= 0.66;
    }

    if (vel_y < 0) {
        this.animation.play("walk_up");
    } else if (vel_y > 0) {
        this.animation.play("walk_down");
    } else if (vel_x < 0) {
        this.animation.play("walk_left");
    } else if (vel_x > 0) {
        this.animation.play("walk_right");
    } else {
        this.animation.stop_all();
    }

    this.pos.x += vel_x;
    this.pos.y += vel_y;
}

// end module
return pm;
}());