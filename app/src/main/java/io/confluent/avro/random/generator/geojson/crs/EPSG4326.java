package io.confluent.avro.random.generator.geojson.crs;

public class EPSG4326 extends CRS {
    public EPSG4326() {
        super();
        double leftBound = -180.0;
        double rightBound = 180.0;
        double bottomBound = 90.0;
        double upperBound = -90.0;
        this.setBounds(leftBound, rightBound, bottomBound, upperBound);
    }

    @Override
    public String toString() {
        return "EPSG4326";
    }
}
