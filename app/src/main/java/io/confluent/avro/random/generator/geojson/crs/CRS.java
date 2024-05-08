package io.confluent.avro.random.generator.geojson.crs;

import java.util.Random;

public abstract class CRS {
    protected double leftBound;
    protected double rightBound;
    protected double bottomBound;
    protected double upperBound;

    protected void setBounds(
            double leftBound,
            double rightBound,
            double bottomBound,
            double upperBound
    ) {
        this.leftBound = leftBound;
        this.rightBound = rightBound;
        this.bottomBound = bottomBound;
        this.upperBound = upperBound;
    }

    public double randomLatitude() {
        Random r = new Random();
        return -90 + r.nextInt(180)  + r.nextDouble();
    }

    public double randomLongitude() {
        Random r = new Random();
        return -180 + r.nextInt(360) + r.nextDouble();
    }

    public abstract String toString();
}
