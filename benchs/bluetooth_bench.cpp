#include <benchmark/benchmark.h>
#include <hj/hardware/bluetooth.h>

#include <vector>
#include <memory>
#include <cstdlib>

// This benchmark file avoids calling hid_enumerate() directly to remain
// hermetic when hidapi isn't available at runtime. Instead we exercise the
// logic in `bluetooth.h` by calling `default_bluetooth_device_filter` on
// many synthetic `bluetooth_info_t` structures and by simulating the
// traversal behavior used in `bluetooth_device_count` and
// `bluetooth_device_range`.

static bluetooth_info_t *make_dummy_device(unsigned short vendor_id,
                                           int            bus_type = 0)
{
    bluetooth_info_t *d = new bluetooth_info_t();
    // zero-initialize commonly used fields to be safe
    std::memset(d, 0, sizeof(bluetooth_info_t));
    d->vendor_id = vendor_id;
    // Some builds expose bus_type; set if available.
#ifdef HID_API_BUS_BLUETOOTH
    d->bus_type = bus_type;
#endif
    d->next = nullptr;
    return d;
}

static void free_dummy_list(bluetooth_info_t *head)
{
    while(head)
    {
        bluetooth_info_t *n = head->next;
        delete head;
        head = n;
    }
}

// Build a linked list of N devices; fraction matching the default filter
// can be controlled via matching_fraction (0.0 - 1.0).
static bluetooth_info_t *build_device_list(size_t n,
                                           double matching_fraction = 0.2)
{
    if(n == 0)
        return nullptr;

    // vendor id candidates from default_bluetooth_device_filter list
    unsigned short vendors[] = {0x0A5C,
                                0x8087,
                                0x0CF3,
                                0x0489,
                                0x05AC,
                                0x046D,
                                0x045E,
                                0x1286,
                                0x0A12,
                                0x04CA,
                                0x0930,
                                0x13D3,
                                0x0BDA};
    const size_t   vcount    = sizeof(vendors) / sizeof(vendors[0]);

    bluetooth_info_t *head = nullptr;
    bluetooth_info_t *tail = nullptr;
    for(size_t i = 0; i < n; ++i)
    {
        // decide whether this entry should match the filter
        bool match =
            (static_cast<double>(std::rand()) / RAND_MAX) < matching_fraction;
        unsigned short vid =
            match ? vendors[i % vcount]
                  : static_cast<unsigned short>((i * 31) & 0xffff);
        int               bus = match ? HID_API_BUS_BLUETOOTH : 0;
        bluetooth_info_t *d   = make_dummy_device(vid, bus);
        if(!head)
            head = tail = d;
        else
        {
            tail->next = d;
            tail       = d;
        }
    }
    return head;
}

// Simulate the behavior of bluetooth_device_count using an existing list.
static int simulated_device_count(const bluetooth_info_t    *head,
                                  bluetooth_device_filter_fn filter)
{
    int count = 0;
    for(const bluetooth_info_t *info = head; info; info = info->next)
    {
        if(filter && !filter(info))
            continue;
        ++count;
    }
    return count;
}

// Simulate the behavior of bluetooth_device_range using an existing list.
static void simulated_device_range(bluetooth_info_t          *head,
                                   bluetooth_device_range_fn  fn,
                                   bluetooth_device_filter_fn filter)
{
    if(!fn)
        return;
    for(bluetooth_info_t *info = head; info; info = info->next)
    {
        if(filter && !filter(info))
            continue;
        if(!fn(info))
            break;
    }
}

// Benchmark: cost of calling default_bluetooth_device_filter on many items
static void bm_default_filter_calls(benchmark::State &st)
{
    const size_t      N    = static_cast<size_t>(st.range(0));
    bluetooth_info_t *head = build_device_list(N, 0.5);
    for(auto _ : st)
    {
        size_t matched = 0;
        for(bluetooth_info_t *it = head; it; it = it->next)
        {
            if(default_bluetooth_device_filter(it))
                ++matched;
        }
        benchmark::DoNotOptimize(matched);
    }
    free_dummy_list(head);
}
BENCHMARK(bm_default_filter_calls)->Arg(128)->Arg(512)->Arg(2048)->Arg(8192);

// Benchmark: simulated device_count traversal and filtering
static void bm_simulated_device_count(benchmark::State &st)
{
    const size_t      N    = static_cast<size_t>(st.range(0));
    bluetooth_info_t *head = build_device_list(N, 0.25);
    for(auto _ : st)
    {
        int c = simulated_device_count(head, default_bluetooth_device_filter);
        benchmark::DoNotOptimize(c);
    }
    free_dummy_list(head);
}
BENCHMARK(bm_simulated_device_count)->Arg(128)->Arg(1024)->Arg(8192);

// Benchmark: simulated device_range with a lightweight callback
static bool trivial_callback(bluetooth_info_t *d)
{
    // return true to continue
    (void) d;
    return true;
}

static void bm_simulated_device_range(benchmark::State &st)
{
    const size_t      N    = static_cast<size_t>(st.range(0));
    bluetooth_info_t *head = build_device_list(N, 0.1);
    for(auto _ : st)
    {
        simulated_device_range(head,
                               trivial_callback,
                               default_bluetooth_device_filter);
        benchmark::ClobberMemory();
    }
    free_dummy_list(head);
}
BENCHMARK(bm_simulated_device_range)->Arg(256)->Arg(2048)->Arg(16384);
