package fncs;

import java.lang.String;

public class JNIfncs {
    static {
        System.loadLibrary("JNIfncs");
    }

    public static void main(String[] args) {
        String config = "name = jcoverage\n"
            + "time_delta = 1s\n"
            + "values\n"
			+ "    fcoverage/key\n"
			+ "    player/key\n"
			+ "    pycoverage/key\n"
			+ "    key_anon\n"
            + "        topic = global/key_anon\n"
            + "        list = true\n";
		int[] version = get_version();
        System.out.printf("jcoverage test running FNCS version %d.%d.%d\n",
                version[0], version[1], version[2]);
        initialize(config);
        assert is_initialized();

        String name = get_name();

        System.out.printf("My name is '%s'\n", name);
        System.out.printf("I am federate %d out of %d other federates\n",
                get_id(), get_simulator_count());

        for (long time=0; time<20; time+=2) {
            long current_time = time_request(time);
            String[] events = get_events();
            System.out.printf("current time is %d, received %d events\n",
                    current_time, events.length);
            System.out.printf("\tevent\tvalue\n");
            for (int i=0; i<events.length; ++i) {
                String value = get_value(events[i]);
                String[] values = get_values(events[i]);
                assert value == values[0];
                for (int j=0; j<values.length; ++j) {
                    System.out.printf("\t[%d] %s\t[%d] %s\n", i, events[i], j, values[j]);
                }
            }
            publish("key", "value");
            publish_anon("global/key_anon", name);
            route("from", "to", "key", "value");
        }

        end();
        assert !is_initialized();
    }

    public static native void initialize();
    public static native void initialize(String configuration);
    public static native void agentRegister();
    public static native void agentRegister(String configuration);
    public static native boolean is_initialized();
    public static native long time_request(long next);
    public static native void publish(String key, String value);
    public static native void publish_anon(String key, String value);
    public static native void agentPublish(String value);
    public static native void route(String from, String to, String key, String value);
    public static native void die();
    public static native void end();
    public static native void update_time_delta(long delta);
    public static native String[] get_events();
    public static native String agentGetEvents();
    public static native String get_value(String key);
    public static native String[] get_values(String key);
    public static native String[] get_keys();
    public static native String get_name();
    public static native long get_time_delta();
    public static native long get_id();
    public static native long get_simulator_count();
    public static native int[] get_version();

}

