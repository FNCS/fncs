import fncs.JNIfncs;

public class jcoverage {
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
		int[] version = JNIfncs.get_version();
        System.out.printf("jcoverage test running FNCS version %d.%d.%d\n",
                version[0], version[1], version[2]);
        JNIfncs.initialize(config);
        assert JNIfncs.is_initialized();

        String name = JNIfncs.get_name();

        System.out.printf("My name is '%s'\n", name);
        System.out.printf("I am federate %d out of %d other federates\n",
                JNIfncs.get_id(), JNIfncs.get_simulator_count());

        for (long time=0; time<20; time+=2) {
            long current_time = JNIfncs.time_request(time);
            String[] events = JNIfncs.get_events();
            System.out.printf("current time is %d, received %d events\n",
                    current_time, events.length);
            System.out.printf("\tevent\tvalue\n");
            for (int i=0; i<events.length; ++i) {
                String value = JNIfncs.get_value(events[i]);
                String[] values = JNIfncs.get_values(events[i]);
                assert value == values[0];
                for (int j=0; j<values.length; ++j) {
                    System.out.printf("\t[%d] %s\t[%d] %s\n", i, events[i], j, values[j]);
                }
            }
            JNIfncs.publish("key", "value");
            JNIfncs.publish_anon("global/key_anon", name);
            JNIfncs.route("from", "to", "key", "value");
        }

        JNIfncs.end();
        assert !JNIfncs.is_initialized();
    }
}

