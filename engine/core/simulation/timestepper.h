#pragma once

class TimeStepper {

	void step() {
		double newTime = time();
		double frameTime = newTime - currentTime;
		if (frameTime > 0.25)
			frameTime = 0.25;
		currentTime = newTime;

		accumulator += frameTime;

		while (accumulator >= dt)
		{
			previousState = currentState;
			integrate(currentState, t, dt);
			//FixedUpdates
			t += dt;
			accumulator -= dt;
		}

		const double alpha = accumulator / dt;

		State state = currentState * alpha +
			previousState * (1.0 - alpha);

		render(state);
		//Updates
	}
};