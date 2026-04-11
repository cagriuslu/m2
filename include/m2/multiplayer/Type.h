#pragma once

namespace m2::mplayer {
    /// \brief Networking model of the multiplayer game.
    /// \details How to choose which model to use? For turn based games, use TurnBased. For real time games, pick
    /// one of the Lockstep variants if a noticeable input lag isn't a problem and determinism in the gameplay is
    /// desired. Pick StateCast if low input lag is crucial, and small errors in the simulation isn't a problem.
    enum class Type {
        /// Only one client is allowed to send a command to the server while all other clients wait. Server validates
        /// the input, advances the simulation, and publishes a full state update to all clients. In this mode, the
        /// physics engine is disabled. This mode is most suitable for board games.
        TurnBased,
        /// Also known as Deterministic Lockstep. Clients send the input events to all peers, including the server,
        /// without immediately simulating them. Following a fixed tick, and if all the input events have been received,
        /// clients advance the simulation locally. The server receives and validates the events as well. Clients relay
        /// some of the events to the server so that the server can validate that clients are sending the same events to
        /// everyone. Before simulation is advanced, all events must be delivered to all clients, thus, lost packets can
        /// easily cause a lag. Possibility of a lost packets increase with the number of players, thus this networking
        /// model isn't suitable for many (4+) players. In this variant, the physics engine is disabled.
        Lockstep,
        /// Same as Lockstep, but the physics engine enabled.
        RealTime,
        /// In this mode, the clients send the input events to the server, but then either predicts the future, or
        /// interpolates the simulation by applying some delay to the incoming state updates. In the meanwhile, the
        /// server validates the events, advances the simulation, then publishes the state to the clients. Once the
        /// state is received on the client side, the client steers the simulation towards the received state, rather
        /// than rolling back the simulation. Since the clients don't wait for events to arrive to advance their
        /// simulation, there is seemingly less input lag, and latency jitters are smoothed over. Since the while state
        /// is published by the server, only a low number of interactive objects can be handled.
        StateCast,
    };
}
