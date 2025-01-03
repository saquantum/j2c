public final class String implements Comparable<String>{
    private final char[] value;
    private final int hash;
    
    public native String(char[] value){}
    
    public native String(String original){}
    
    public native int length(){}
    
    public native boolean isEmpty(){}
    
    public native char charAt(int index){}
    
    public native String substring(int beginIndex, int endIndex){}
    
    public native boolean equals(String that){}
    
    @override
    public native int compareTo(String that){}
    
    public native char[] toCharArray(){}
    
    @override
    public native int hashCode(){}
    
    
}
