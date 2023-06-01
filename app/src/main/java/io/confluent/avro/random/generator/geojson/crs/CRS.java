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
        return r.nextDouble() * ((upperBound - bottomBound) + bottomBound);
    }

    public double randomLongitude() {
        Random r = new Random();
        return r.nextDouble() * ((rightBound - leftBound) + rightBound);
    }

    public abstract String toString();
}
