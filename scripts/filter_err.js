function filterError(errString='') {
  let pattern = /(?:ParseError\(.*"(.+)"\))[\s\S]*(?:array subscript value '([0-9]+)')/;
  let groups = errString.match(pattern);
  if (groups) 
  {
    return `${groups[1]} at line: ${groups[2]}`;
  }
  return errString;
}

if (!process.argv[2])
  console.log('');
else
  console.log(filterError(process.argv[2]));
