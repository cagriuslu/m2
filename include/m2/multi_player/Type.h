#pragma once

namespace m2::mplayer {
    /// How to choose which networking model to use? If input lag is acceptable and the game has low number of players,
    /// prefer Lockstep, as it's the most efficient one. If input lag is unacceptable or the game has high number of
    /// players (4+), Rollback or StateCast are the only options as those utilize client-side prediction to hide the
    /// input lag, or to smooth over the latency spikes. If levels contain many interactive objects (hundreds) and the
    /// object don't collide with each other excessively, use Rollback, as it scales better with increasing number of
    /// objects. If levels contain few moving objects, use StateCast as it handles latency spikes well.
    enum class Type {
        /// Only one client is allowed to send a command to the server while all other clients wait. Server validates
        /// the input, advances the simulation, and publishes a full state update to all clients. In this mode, the
        /// physics subsystem is disabled. This mode is most suitable for board games.
        TurnBased,
        /// Also known as Deterministic Lockstep. Clients send the input events to the server without simulating them
        /// locally. Server verifies the events, and streams them to all clients. Clients wait for the events to arrive
        /// before advancing their simulation, thus, even in the best network conditions, clients experience input lag.
        /// If the network conditions worsen, all players are effected, thus this networking strategy is not suitable
        /// for many (4+) players. No client-size prediction is made, thus no rollback or correction is necessary.
        Lockstep,
        /// Same as Lockstep, but the physics engine isn't initialized.
        LockstepNoPhysics,
        /// In this mode, the clients send the input events to the server, but then either predicts the future, or
        /// interpolates the simulation by applying some delay to the incoming inputs. In the meanwhile, the server
        /// validates the events, advances the simulation, then streams the inputs to the clients. Once the events are
        /// received on the client side, the client reloads the state of the game where the last input was received from
        /// the server, and advances the simulation based on real data. Since the clients don't wait for events to
        /// arrive to advance their simulation, there is seemingly less input lag, and latency jitters are smoothed
        /// over. If the client is forced to rollback from too earlier, or rollback takes long time to catch up, the
        /// client might spiral into unrecoverable state constantly trying to catch up.
        Rollback, // TODO the down side of this mode is pretty severe, should we implement this at all?
        /// In this mode, the clients send the input events to the server, but then either predicts the future, or
        /// interpolates the simulation by appliying some delay to the incoming state updates. In the meanwhile, the
        /// server validates the events, advances the simulation, then publishes the state to the clients. Once the
        /// state is received on the client side, the client steers the simulation towards the received state, rather
        /// than rolling back the simulation. Since the clients don't wait for events to arrive to advance their
        /// simulation, there is seemingly less input lag, and latency jitters are smoothed over. Since the while state
        /// is published by the server,  only a low number of interactive objects can be handled.
        StateCast,
    };
}
