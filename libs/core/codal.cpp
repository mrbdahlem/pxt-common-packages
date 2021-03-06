#include "pxt.h"

void cpu_clock_init(void);

namespace pxt {

void platform_init();
void usb_init();

// The first two word are used to tell the bootloader that a single reset should start the
// bootloader and the MSD device, not us.
// The rest is reserved for partial flashing checksums.
__attribute__((section(".binmeta"))) __attribute__((used)) const uint32_t pxt_binmeta[] = {
    0x87eeb07c, 0x87eeb07c, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
    0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
};

CODAL_MBED::Timer devTimer;
Event lastEvent;
MessageBus devMessageBus;
codal::CodalDevice device;

static void initCodal() {
    cpu_clock_init();

    // Bring up fiber scheduler.
    scheduler_init(devMessageBus);

    // We probably don't need that - components are initialized when one obtains
    // the reference to it.
    // devMessageBus.listen(DEVICE_ID_MESSAGE_BUS_LISTENER, DEVICE_EVT_ANY, this,
    // &CircuitPlayground::onListenerRegisteredEvent);

    for (int i = 0; i < DEVICE_COMPONENT_COUNT; i++) {
        if (CodalComponent::components[i])
            CodalComponent::components[i]->init();
    }

    usb_init();
}

// ---------------------------------------------------------------------------
// An adapter for the API expected by the run-time.
// ---------------------------------------------------------------------------

// We have the invariant that if [dispatchEvent] is registered against the DAL
// for a given event, then [handlersMap] contains a valid entry for that
// event.
void dispatchEvent(Event e) {
    lastEvent = e;

    auto curr = findBinding(e.source, e.value);
    if (curr)
        runAction1(curr->action, fromInt(e.value));

    curr = findBinding(e.source, DEVICE_EVT_ANY);
    if (curr)
        runAction1(curr->action, fromInt(e.value));
}

void registerWithDal(int id, int event, Action a, int flags) {
    // first time?
    if (!findBinding(id, event))
        devMessageBus.listen(id, event, dispatchEvent, flags);
    setBinding(id, event, a);
}

void fiberDone(void *a) {
    decr((Action)a);
    release_fiber();
}

void sleep_ms(unsigned ms) {
    fiber_sleep(ms);
}

void sleep_us(uint64_t us) {
    wait_us(us);
}

void forever_stub(void *a) {
    while (true) {
        runAction0((Action)a);
        fiber_sleep(20);
    }
}

void runForever(Action a) {
    if (a != 0) {
        incr(a);
        create_fiber(forever_stub, (void *)a);
    }
}

void runInBackground(Action a) {
    if (a != 0) {
        incr(a);
        create_fiber((void (*)(void *))runAction0, (void *)a, fiberDone);
    }
}

void waitForEvent(int id, int event) {
    fiber_wait_for_event(id, event);
}

void initRuntime() {
    initCodal();
    platform_init();
}

//%
unsigned afterProgramPage() {
    unsigned ptr = (unsigned)&bytecode[0];
    ptr += programSize();
    ptr = (ptr + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
    return ptr;
}

int getSerialNumber() {
    return device.getSerialNumber();
}

int current_time_ms() {
    return system_timer_current_time();
}
}
