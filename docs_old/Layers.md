# Layers

The engine provides different layers for different purposes.
They can be categorized as follows:
- Physics Layers
- Background Draw Layer
- Foreground Draw Layer

## Physics Layers

Each physics layer represents a completely independent physical world.
After a Physique component is created, the rigid body belonging to the component can be placed in any of the Physics layers.
Bodies in different layers do not collide with each other.

Currently, there are 3 physics layers:
- P0 is the default layer. It's analogous to the ground level in a top-down game.
- P1 is the upper layer. It's analogous to above ground level in a top-down game.
- PM1 is the lower layer. It's analogous to underground level in a top-down game.

So far, there are 2 use-cases for physics layers:
1. An object might move between layers as if it changes its height in a top-down game, or changes its distance to screen in a platformer.
2. An object might need to exist at a different layer than the default one, ex. a plane, a submarine.

## Background Draw Layers

Background draw layers represent background graphics.
They do not utilize a DrawList, thus the objects within a layer aren't drawn with a predicable order.
For best results, non-overlapping sprites should be used in these layers.
In 3D mode, background layers are drawn parallel to the ground.

There are 4 of them:
- B0 is the front-most layer, it's drawn the last among the background layers.
- B1
- B2
- B3 is the back-most layer, it's drawn the first.

## Foreground Draw Layers

Foreground draw layers represent the graphics of the interactive objects.
They maintain separate DrawLists, thus the objects are sorted before they are drawn.
In 3D mode, foreground layers are drawn perpendicular to the ground.

There are 6 of them. Although it's not enforced, each foreground draw layer pair corresponds to a physics layer:
- F0_BOTTOM is the default layer.
- F0_TOP
- F1_BOTTOM
- F1_TOP
- FM1_BOTTOM
- FM1_TOP

# Draw Order

The draw layers are drawn in this order:
- B3, B2 are the back-most graphics. Ex. graphics of an ocean floor.
- FM1_BOTTOM, FM1_TOP are the first layer of foreground graphics. These graphics usually have their rigid bodies in the PM1 physics layer. Ex. a submarine.
- B1, B0 are the top-most background graphics. Ex. graphics of a ship dock.
- F0_BOTTOM, F0_TOP. These graphics usually have their rigid bodies in the P0 physics layer. Ex. a soldier on foot.
- F1_BOTTOM, F1_TOP are the front-most graphics. These graphics usually have their rigid bodies in the P1 physics layer. Ex. a plane flying in the air.
