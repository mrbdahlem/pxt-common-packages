# pause

Pause a part of the program for some number of milliseconds.

```sig
loops.pause(400)
```

When code in a block comes to a ``||control:pause||``, it will wait the amount of time you tell it to. Code
in blocks like ``||forever||`` and ``||control:run in background||`` will keep running while code in some other
block is waiting at a ``||control:pause||``.

## Parameters

* **ms**: the [number](/types/number) of milliseconds that you want to pause for. So, 100 milliseconds = 1/10 second, and 1000 milliseconds = 1 second.

## Example

Light up to `5` pixels but wait one-half second before lighting each pixel.

```blocks
let pixels = light.createStrip();
for (let i = 0; i < 5; i++) {
    pixels.setPixelColor(i, Colors.Green)
    loops.pause(500)
}
```

## See also

[``||while||``](/blocks/loops/while), [``||for||``](/blocks/loops/for),
[``||forever||``](/reference/loops/forever)
