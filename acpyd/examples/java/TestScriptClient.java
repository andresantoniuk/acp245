import java.net.*;
import java.io.*;

/*
 * Sample Java code to interact with the ACP Test Server HTTP Interface.
 *
 * @author Edantech.
 */
public class TestScriptClient {
    /* default URL path */
    public static String PATH = "http://acptest.edantech.net/gateway/";
    private static String ip = "127.0.0.1";
    private static String port = "10101";
    private static String user = "edantech";
    private static String password = "3d4nt3ch";
    /*
    public static String PATH = "http://porto.acptest.edantech.net/gateway/";
    private static String ip = "201.76.47.53";
    private static String port = "3014";
    private static String user = "porto";
    private static String password = "darcyin6";
    */
    public static String SUFFIX = "";

    public static void main(String[] args) throws Exception {
        parseArguments(args);

        String resp;

        /* start test case */
        resp = getResponse("start_test?name=first_test");
        /*
         * The response should be checked here to see if it's as expected.
         * In this example we just print it to standard output.
         */
        System.out.println("Response: " + resp);

        /* Stop the client in case is already running */
        resp = getResponse("client/stop");
        System.out.println("Response: " + resp);

        /* start client */
        String req = "client/start?ip=" + ip + "&port=" + port;
        resp = getResponse(req);
        System.out.println("Response: " + resp);

        /* wait for connection to the server, timeout 10 seconds */
        resp = getResponse("client/wait_for_connection?" +
                "timeout=10");
        System.out.println("Response: " + resp);

        /* send an alarm notification, timeout 60 seconds */
        resp = getResponse("client/send_alarm_notif?" +
                "sim_card_id=12345678901234567890" +
                "&hdr_msg_ctrl=RESP_EXP" +
                "&timeout=60");
        System.out.println("Response: " + resp);

        /* wait for an alarm reply */
        resp = getResponse("client/pop_alarm_reply?"+
                "timeout=60");
        System.out.println("Response: " + resp);

        /* wait for a function command */
        resp = getResponse("client/wait_for_func_cmd?"+
                "function_status=ENABLED" + /* command enabled */
                "&entity_id=128" +          /* 128 = ACP245 Blocking */
                "&timeout=60");             /* timeout for response */
        System.out.println("Response: " + resp);

        /* close the client connection */
        resp = getResponse("client/stop");
        System.out.println("Response: " + resp);

        /* Its also a good idea to stop the test case. */
        resp = getResponse("stop_test");
        System.out.println("Response: " + resp);
    }

    private static void parseArguments(String[] args) {
        /* a command line argument can be provided as the URL prefix */
        if (args.length > 0) {
            PATH = args[0] + "/gateway/";
        }
        if (args.length > 1) {
            ip = args[1];
        }
        if (args.length > 2) {
            port = args[2];
        }
        if (args.length > 3) {
            user = args[3];
        }
        if (args.length > 4) {
            password = args[4];
        }
        SUFFIX += "&user=" + user;
        SUFFIX += "&password=" + password;
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
