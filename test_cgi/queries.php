<?php
    $queries = array();
    parse_str($_SERVER['QUERY_STRING'], $queries);
    if(!empty($queries['test']))
      echo $queries['test']."<br>";
    if(!empty($queries['name']))
      echo $queries['name'];
?>