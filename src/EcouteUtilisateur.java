package src;
import java.net.*;
import java.util.*;

import src.Diffuseur;

import java.io.*;
import java.lang.*;

public class EcouteUtilisateur implements Runnable{
    private Socket socket;
    private DiffuseMulticast liveStream;
    private Diffuseur parent;

    public EcouteUtilisateur(Socket s, DiffuseMulticast live){
        this.socket = s;
        this.liveStream = live;
    }

    public void setDiffuseur(Diffuseur d){
        this.parent = d;
    }

    public String[] traitementRequete(String s){
        String [] stock;
        if(s.substring(0, 4).equals(Diffuseur.LAST)){
            stock = new String[2];
            stock[0] = s.substring(0, 4);
            stock[1] = s.substring(5, 8);
        } else if (s.substring(0, 4).equals(Diffuseur.DIFF)){
            stock = new String[3];
            stock[0] = s.substring(5, 9);
            stock[1] = s.substring(10, 18);
            stock[2] = s.substring(19, s.length()-2);
        } else {
            stock = new String[3];
            stock[0] = s.substring(0, 4);
            stock[1] = s.substring(5, 13);
            stock[2] = s.substring(14, s.length()-2);
        }
 
        return stock;
    }

    public void receptionLast(PrintWriter pw, String [] traitement){
        try{
            int nbMsg = Integer.valueOf(traitement[1]);
            int posMsg = this.liveStream.getIndice();

            if(nbMsg > this.liveStream.getEnvoye()){
                nbMsg = this.liveStream.getEnvoye();
            }

            for(int i = 0; i < nbMsg; i++){
                if (posMsg == -1){
                    posMsg = this.liveStream.getListMsg().size() - 1;
                }

                String [] ancienMsg = traitementRequete(this.liveStream.getListMsg().get(posMsg));

                pw.print(Diffuseur.OLDM + " " + ancienMsg[0] + " " + ancienMsg[1] + " " + ancienMsg[2] + "\r\n");
                pw.flush();
                posMsg--;
            }
            pw.print(Diffuseur.ENDM + "\r\n");
            pw.flush();
        } catch(NumberFormatException e) {
            pw.print("[Erreur] : Format de nombre invalide\n");
            pw.flush();
        }
    }

    public void run(){
        try{
            while(true){
                BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));

                String msg = br.readLine();

                if(msg == null){
                    continue;
                }

                String [] traitement = traitementRequete(msg);

                if (traitement[0].equals(Diffuseur.MESS) 
                    && traitement[1].length() <= Diffuseur.TAILLEID
                    && traitement[2].length() <= Diffuseur.TAILLEMAXMSG) {
                    liveStream.getListMsg().add(parent.assembleMsgDiff(traitement[1], traitement[2]));

                    pw.print(Diffuseur.ACKM + "\r\n");
                    pw.flush();

                    this.socket.close();
                    break;
                } else if (traitement[0].equals(Diffuseur.LAST)) {
                    receptionLast(pw, traitement);
                } else if (traitement[0].equals(Diffuseur.RUOK)) {
                    pw.print(Diffuseur.IMOK + "\r\n");
                    pw.flush();
                } else {
                    pw.print("[Erreur] : Message au mauvais format\n");
                    pw.flush();
                }
            }
        } catch(Exception e) {
            System.out.println(e);
            e.printStackTrace();
        }
    }
}
