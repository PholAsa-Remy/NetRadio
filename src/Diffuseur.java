import java.io.*;
import java.net.*;
import java.util.*;

public class Diffuseur{
    private String identifiant;
    private int portMsg;
    private InetSocketAddress multiDiff;
    private int portMultiDiff;
    private LinkedList<String> listMsg;
    /*private DiffuseMulticast live;
    private EcouteUtilisateur ecoute;*/

    public int numMsg = 0;
    public static final String ACKM = "ACKM";
    public static final String ENDM = "ENDM";
    public static final String LAST = "LAST";
    public static final String DIFF = "DIFF";
    public static final String MESS = "MESS";
    public static final String OLDM = "OLDM";
    public static final String REGI = "REGI";
    public static final String RUOK = "RUOK";
    public static final String IMOK = "IMOK";
    public static final int TAILLEMAXMSG = 140;
    public static final int TAILLEID = 8;
    public static final int FORMATPORT = 9999;

    public Diffuseur(String id, int portRecv, int portDiff){
        if (portDiff > FORMATPORT || portRecv > FORMATPORT || id.length() > TAILLEID){
            System.out.println("[Erreur] : Impossible d'assembler le diffuseur, rappel : le numéro des ports doit être inférieur à 9999 et l'identifiant ne doit faire plus de 8 caractères\n");
            System.exit(1);
        }
        
        this.identifiant = id;
        this.portMsg = portRecv;
        this.multiDiff = new InetSocketAddress(portDiff);
        this.portMultiDiff = portDiff;
        this.listMsg = new LinkedList<String>();
    }

    /*public void setEcoute(EcouteUtilisateur eu){
        this.ecoute = eu;
    }*/

    /*public Diffuseur(String id, DatagramSocket recv, InetSocketAddress multiDiff, DatagramSocket portDiff) throws Exception{
        if (id.length() > 8){
            throw new Exception("[Erreur] : Impossible d'assembler le diffuseur, rappel : l'identifiant ne doit faire plus de 8 caractères\n");
        }
        
        this.identifiant = id;
        this.portMsg = recv;
        this.multiDiff = multiDiff;
        this.portMultiDiff = portDiff;
        this.listMsg = new LinkedList<String>();
    }*/

    //TODO: bricoler l'affichage en 4 bits du numéro
    public String assembleMsgDiff(String id, String message){
        if (message.length() > TAILLEMAXMSG){
            return "";
        }

        if (numMsg == 9999){
            numMsg = 0;
        } else {
            numMsg++;
        }

        return DIFF + " " + numMsg + " " + id + " " + message + "\r\n";
    }

    public String assembleMsgEnregistrement(InetSocketAddress ip2){
        return REGI + " " + identifiant + " " + multiDiff + " " + portMultiDiff + " " + ip2 + " " + portMsg + "\r\n";
    }

    public void addToList(String msg){
        this.listMsg.add(msg);
    }

    public static int stringToInt(String s){
        int i = -1;
        try{
            i = Integer.valueOf(s);
        } catch (NumberFormatException n){
            System.out.println("[Erreur] : Le nombre n'est pas au bon format");
            System.exit(1);
        }
        return i;
    }

    /*Champs supposés de args : 
     * [0] : ID
     * [1] : port multidiffusion
     * [2] : port écoute
     * [3] : adresse multidiffusion
     * [4] : port gestionnaire
     */
    public static void main(String [] args){
        if(args.length < 5){
            System.out.println("[Erreur] : Pas assez d'arguments");
        }

        LinkedList<String> msgDiff = new LinkedList<String>();

        DiffuseMulticast dm = new DiffuseMulticast(msgDiff, stringToInt(args[1]), args[3]);
        Diffuseur d = new Diffuseur(args[0], stringToInt(args[2]), stringToInt(args[1]));
        EcouteUtilisateur eu = null;
        dm.setDiffuseur(d);

        dm.ajoutMsg(d.assembleMsgDiff(args[0], "The apocalypse shall soon be realised..."));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "Are there any grounds for that boldness of yours?"));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "Oh...?"));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "I've no intention of stopping here."));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "Cast away that flesh, onward to the next dimension..."));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "I'll be using your power. Paradise Lost!"));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "I wonder if I can still draw out more efficiency..."));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "Be seated, and await your end!"));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "Tremble before the oblivion..."));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "Still putting up a fight...?!"));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "Even the power of these twelve black wings...?!"));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "What an irritating lot..."));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "Is this no longer useful..."));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "This doesn't bode well."));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "Have I been surpassed...?!"));
        dm.ajoutMsg(d.assembleMsgDiff(args[0], "So I have erred in my calculations..."));
        
        try{
            ServerSocket server = new ServerSocket(stringToInt(args[2]));
            Socket socket = server.accept();
            eu = new EcouteUtilisateur(socket, dm);
            eu.setDiffuseur(d);
            Thread ecoute = new Thread(eu);
            Thread stream = new Thread(dm);
            ecoute.start();
            stream.start();
        } catch(Exception e) {
            System.out.println(e);
            e.printStackTrace();
        }
    }
}