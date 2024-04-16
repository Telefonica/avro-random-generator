package com.telefonica.baikal.utils.avro;

public class NotInformedUtilsBridge implements NotInformedUtils {

    private static NotInformedUtilsBridge instance = null;

    public static synchronized NotInformedUtilsBridge getInstance() {
       if (instance == null) {
           instance = new NotInformedUtilsBridge();
           return instance;
       }
       return instance;
    }

}
