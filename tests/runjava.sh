javac -classpath ".:../java" jcoverage.java
# java -classpath ".:../java" -Djava.library.path=".:../java" jcoverage
(export FNCS_LOG_STDOUT=yes && export FNCS_LOG_LEVEL=DEBUG4 && exec fncs_broker 1 &> broker.log &)
(export FNCS_LOG_STDOUT=yes && export FNCS_LOG_LEVEL=DEBUG4 && exec java -classpath ".:../java" -Djava.library.path=".:../java" jcoverage &> java.log &)

