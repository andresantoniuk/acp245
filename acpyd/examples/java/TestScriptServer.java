import java.net.*;
import java.io.*;

/*
 * Sample Java code to interact with the ACP Test Server HTTP Interface.
 *
 * @author Edantech.
 */
public class TestScriptServer {
    /* default URL path */
    public static String PATH = "http://acptest.edantech.net/gateway/";
    public static String SUFFIX = "";

    public static void main(String[] args) throws Exception {
        /* a command line argument can be provided as the URL prefix */
        if (args.length > 0) {
            PATH = args[0] + "/gateway/";
        }
        String port = null;
        if (args.length > 1) {
            port = args[1];
        }
        if (args.length > 2) {
            SUFFIX += "&user=" + args[2];
        }
        if (args.length > 3) {
            SUFFIX += "&password=" + args[3];
        }

        String resp = null;
        /* start test case */
        resp = getResponse("start_test?name=first_test");

        /*
         * The response should be checked here to see if it's as expected.
         * In this example we just print it to standard output.
         */
        System.out.println("Response: " + resp);

        /* start server */
        if (port != null) {
            resp = getResponse("server/start?port=" + port);
        } else {
            resp = getResponse("server/start");
        }
        System.out.println("Response: " + resp);

        /* wait for client connection, timeout 10 seconds */
        resp = getResponse("server/wait_for_connection?" +
                "timeout=10");
        System.out.println("Response: " + resp);
        /* wait for alarm notification form client, timeout 60 seconds */
        resp = getResponse("server/pop_alarm_notif?"+
                "timeout=60");
        System.out.println("Response: " + resp);

        /* send alarm reply */
        resp = getResponse("server/send_alarm_reply?"+
                "timeout=60");
        System.out.println("Response: " + resp);

        /* send function command to client,
         * enable entity_id 128 (ACP245 Blocking) */
        resp = getResponse("server/send_func_cmd?" +
                "function_command=ENABLE" + /* command enable */
                "&entity_id=128" +          /* 128 = ACP245 Blocking */
                "&timeout=60");             /* timeout for response */
        System.out.println("Response: " + resp);

        /* Always remember to stop server after test.
         * It can also be done from the web interface, or performed when
         * starting the test, just to be sure that is stopped before trying
         * to start it */
        /*
        resp = getResponse("server/stop");
        System.out.println("Response: " + resp);
        */

        /* Its also a good idea to stop the test case. */
        resp = getResponse("stop_test");
        System.out.println("Response: " + resp);
    }

    private static String getResponse(String action) throws Exception {
        URL page = new URL(PATH + action + (action.contains("?") ?  SUFFIX : "?" + SUFFIX));

        System.out.println("Connecting to " + page);

        HttpURLConnection conn = (HttpURLConnection) page.openConnection();
        if (conn.getResponseCode() != HttpURLConnection.HTTP_OK) {
            /* An HTTP error code implies an error on the ACP Test Server.
             * Report it to Edantech as soon as possible so we can provide
             * you with a fix.*/
            System.err.println("Connection failed:" +
                    conn.getResponseMessage());
            System.exit(1);
        }

        BufferedReader br = null;
        try {
            br = new BufferedReader(new InputStreamReader(
                        conn.getInputStream()));
            /* The result is URL encoded, decode it so it's
             * more human-readable */
            return URLDecoder.decode(br.readLine(), "UTF-8");
        } finally {
            if (br != null) {
                br.close();
            }
        }
    }
}
